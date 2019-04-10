####
# Get Module Name:
#
# Takes a path, or something path-like and returns the module's name. This breaks down as the
# following:
#
#  1. If passed a path, the module name is the _ed variant of the relative path from BUILD_ROOT
#  2. If passes something which does not exist on the file system, it is just _ed
#
# i.e. ${BUILD_ROOT}/Svc/ActiveLogger becomes Svc_ActiveLogger
#      Svc/ActiveLogger also becomes Svc_ActiveLogger
#
# \param DIRECTORY_PATH: path to infer MODULE_NAME from
# \param UT: "TRUE" if unit-test varian "FALSE" otherwise
# \return MODULE_NAME (set in parent scope
####
function(get_module_name DIRECTORY_PATH)
  # If DIRECTORY_PATH exists, then find its offset from BUILD_ROOT to calculate the module
  # name. If it does not exist, then it is assumed to be an offset already and is carried
  # forward in the calculation.
  if (EXISTS ${DIRECTORY_PATH})
      # Get path name relative to the root directory
      file(RELATIVE_PATH TEMP_MODULE_NAME ${FPRIME_CURRENT_BUILD_ROOT} ${DIRECTORY_PATH})
  else()
      set(TEMP_MODULE_NAME ${DIRECTORY_PATH})
  endif()
  # Replace slash with underscore to have valid name
  string(REPLACE "/" "_" TEMP_MODULE_NAME ${TEMP_MODULE_NAME})
  if (DEFINED BUILD_SUFFIX)
      set(TEMP_MODULE_NAME "${TEMP_MODULE_NAME}_${BUILD_SUFFIX}")
  endif()
  set(MODULE_NAME ${TEMP_MODULE_NAME} PARENT_SCOPE)
endfunction(get_module_name)

####
# Setup Module Dicts:
#
# Creates a dictionary target for the module, that is then added to the "dict" and "module"
# targets.
# \param MOD_NAME: name of module being processed
# \param AI_XML: AI_XML that is generating dictionaries
# \param DICT_INPUTS: inputs from auto-coder, used to trigger dictionary generation
####
function(setup_module_dicts MOD_NAME AI_XML DICT_INPUTS)
    # UTs don't supply directories
    if (UT_BUILD)
        return()
    endif()
	set(AI_DICT_NAME "${AI_XML}_DICT")
	set(MOD_DICT_NAME "${MOD_NAME}_DICT")
	# Add the dictionary target for this module, if it doesn't already exist
	if (NOT TARGET ${MOD_DICT_NAME})
	    add_custom_target(${MOD_DICT_NAME})
		if (CMAKE_DEBUG_OUTPUT)
		    message(STATUS "\tAdding Dict Target: ${OLD_MODULE_NAME}_DICT")
		endif()
	endif()
	add_custom_target(${AI_DICT_NAME} DEPENDS ${DICT_INPUTS})
	# Add dependencies upstream
	add_dependencies(${AI_DICT_NAME} ${CODEGEN_TARGET})
	add_dependencies(${MOD_DICT_NAME} ${AI_DICT_NAME})
endfunction(setup_module_dicts)

####
# Add Dict Deps:
#
# Used to track dictionary dependencies, in order to ensure that the fewest number of dictionary
# targets are used.
# \param MODULE_NAME: module receiving a dict dependency
# \param DEP_MODULE_NAME: name of the module whose dictionary will be added
####
function(add_dict_deps MODULE_NAME DEP_MODULE_NAME)
    # UTs don't do dictionaries
    if (UT_BUILD)
        return()
    endif()
    # Skip if there is no dict module to be added
    if (TARGET "${DEP_MODULE_NAME}_DICT")
        # We have sub-dictionaries, create a roll-up target
        if (NOT TARGET "${MODULE_NAME}_DICT")
            if (CMAKE_DEBUG_OUTPUT)
                message(STATUS "\tAdding Faux-Dict Target: ${MODULE_NAME}_DICT")
            endif()
            add_custom_target("${MODULE_NAME}_DICT")
        endif()
        add_dependencies("${MODULE_NAME}_DICT" "${DEP_MODULE_NAME}_DICT")
    endif()
endfunction(add_dict_deps)

####
# Generated Files:
#
# Add generated file as sources for the given target. This function specifically adds
# these sources as generated, so that regardless of where they appear (source or build tree)
# they will be known as generated
#
# \param CPP_SOURCE: source .cpp file to add
# \param HPP_SOURCE: source .hpp file to add
# \param MODULE_NAME: name of the module to add generated sources too
####
function(add_generated_sources CPP_SOURCE HPP_SOURCE)
  if (CMAKE_DEBUG_OUTPUT)
    message(STATUS "\tGenerated files: ${CPP_SOURCE} ${HPP_SOURCE}")
  endif()
  set_source_files_properties(${HPP_SOURCE} PROPERTIES GENERATED TRUE)
  # Add auto-coded sources to the module as sources
  target_sources(
    ${MODULE_NAME}
    PRIVATE ${CPP_SOURCE} ${HPP_SOURCE}
  )
  # Set those files as generated to prevent build errors
  set_source_files_properties(${CPP_SOURCE} PROPERTIES GENERATED TRUE)
  set_source_files_properties(${HPP_SOURCE} PROPERTIES GENERATED TRUE)
  # Includes the source, so that the Ac files can include source headers
  target_include_directories("${MODULE_NAME}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction(add_generated_sources)

####
# FPrime Dependencies:
#
# A function used to detect the dependencies of a given module from the XML file that
# defines this module. This is used to reduce code in the Serializable, Port, Component,
# and Topology functions that all use the same procedure.
#
# \param XML_PATH: full path to the XML used for sources.
# \param MODULE_NAME: name of the module soliciting new dependencies
# \param PARSER_TYPE: type of parser to use. Must be one of the prefixes *_xml in cmake/parser/
####
function(fprime_dependencies XML_PATH MODULE_NAME PARSER_TYPE)
  execute_process(
      COMMAND "${FPRIME_CORE_DIR}/cmake/support/parser/ai_parser.py" "${XML_PATH}" "${MODULE_NAME}" "${FPRIME_CURRENT_BUILD_ROOT}"
	  RESULT_VARIABLE ERR_RETURN
	  OUTPUT_VARIABLE TARGETS
  )
  # Check parser return code
  if(ERR_RETURN)
     message(FATAL_ERROR "Failed to parse ${XML_PATH}. ${ERR_RETURN}")
  endif()
  # For every dected dependency, add them to the supplied module. This enforces build order.
  # Also set the link dependencies on this module. CMake rolls-up link dependencies, and thus
  # this prevents the need for manually specifying link orders.
  add_dependencies(${MODULE_NAME} ${TARGETS})
  target_link_libraries(${MODULE_NAME} ${TARGETS})
  
  #Add in all dictionary targets
  foreach(TARGET ${TARGETS})
    add_dict_deps(${MODULE_NAME}  ${TARGET})
  endforeach()
endfunction(fprime_dependencies)

####
# split_source_files:
#
# Split SOURCE_FILE_INPUTS into the autocoder and source variants, supplied to the calling function
####
function(split_source_files SOURCE_INPUT_FILES)
    set(AC "")
    set(SC "")
    foreach (INPUTFILE ${SOURCE_INPUT_FILES})
        if (INPUTFILE MATCHES ".*\.xml$" OR INPUTFILE MATCHES ".*\.txt")
            list(APPEND AC ${INPUTFILE})
        else()
            list(APPEND SC ${INPUTFILE})
        endif()
    endforeach()
	# Return the variables to calling scope
	set(AUTOCODER_INPUT_FILES "${AC}" PARENT_SCOPE)
	set(SOURCE_FILES "${SC}" PARENT_SCOPE)
endfunction(split_source_files)

####
# split_dependencies:
#
# Split DEPS_INPUT into the module and link variants, supplied to the calling function.
####
function(split_dependencies DEPS_INPUT)
    set(LD "")
    set(FD "")
    foreach (INPUTFILE ${DEPS_INPUT})
        if (INPUTFILE MATCHES "^-l.*")
            list(APPEND LD ${INPUTFILE})
        else()
            list(APPEND FD ${INPUTFILE})
        endif()
    endforeach()
	# Return the variables to calling scope
	set(LINK_DEPS "${LD}" PARENT_SCOPE)
	set(MOD_DEPS "${FD}" PARENT_SCOPE)
endfunction(split_dependencies)


####
# Print Dependencies:
#
# Prints the dependency list of the module supplied.
####
function(print_dependencies MODULE_NAME)
     get_target_property(OUT "${MODULE_NAME}" LINK_LIBRARIES)
     if (OUT MATCHES ".*-NOTFOUND")
       set(OUT "--none--")
     endif()
     message(STATUS "\tLinks dependencies: ${OUT}")
endfunction(print_dependencies)

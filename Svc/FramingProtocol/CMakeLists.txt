####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/DeframingProtocol.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/FramingProtocol.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/FprimeProtocol.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/CustomProtocol.cpp"
)

set(MOD_DEPS
    Fw/Buffer
    Fw/Time
    Utils/Hash
    Utils/Types
    Fw/Ports/Node
)

register_fprime_module()

set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/DeframingTester.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/FramingTester.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
)
set(UT_MOD_DEPS STest)
register_fprime_ut()

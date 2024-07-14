####
# CMakeLists.txt:
#
# Sets up the fprime module build within CMake.
####
set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/RetPacket.cpp"
)

set(MOD_DEPS
     Fw/Com
 )

register_fprime_module()
# STEP 2: Specify the target system's name.
# You can use a more descriptive name if needed
set(CMAKE_SYSTEM_NAME "Linux")

# STEP 3: Specify the path to C and CXX cross compilers
# Adjust these paths based on where your cross-compiler is installed
set(CMAKE_C_COMPILER "/usr/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabihf-g++")

# STEP 4: Specify paths to root of toolchain package, for searching for
# libraries, executables, etc.
# This is typically where the sysroot or toolchain libraries are located
set(CMAKE_FIND_ROOT_PATH "/usr/arm-linux-gnueabihf")

# DO NOT EDIT: F prime searches the host for programs, not the cross
# compile toolchain
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# DO NOT EDIT: F prime searches for libs, includes, and packages in the
# toolchain when cross-compiling.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

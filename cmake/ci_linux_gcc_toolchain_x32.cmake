set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_AR ar)

set(CMAKE_CROSSCOMPILING ON)

set(LINUX_X64_TO_X32 1)

set(CMAKE_FIND_ROOT_PATH /usr/lib/i386-linux-gnu/)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


set(CMAKE_CXX_STANDARD 14)

add_library(test_common INTERFACE)
target_include_directories(test_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${TheXTech_SOURCE_DIR}/include
    ${TheXTech_SOURCE_DIR}/src
)

add_subdirectory(test_msg_macro)

add_library(Catch-objects OBJECT "common/catch_amalgamated.cpp")
target_include_directories(Catch-objects PRIVATE "common")

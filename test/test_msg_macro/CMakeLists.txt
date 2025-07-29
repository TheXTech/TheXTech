set(CMAKE_CXX_STANDARD 14)

add_executable(TestMsgMacro
    ${TheXTech_SOURCE_DIR}/src/script/msg_macros.cpp
    ${TheXTech_SOURCE_DIR}/src/script/msg_macros.h
    test_msg_macro.cpp
    $<TARGET_OBJECTS:Catch-objects>
)
target_link_libraries(TestMsgMacro PRIVATE test_common)
target_compile_definitions(TestMsgMacro PRIVATE -DMOONDUST_UNIT_TEST)
add_test(NAME TestMsgMacro COMMAND ConversionTest)

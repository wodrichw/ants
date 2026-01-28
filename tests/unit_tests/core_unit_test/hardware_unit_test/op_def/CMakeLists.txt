add_executable(test_hardware_op_def
    test_op_def.cpp
)

target_link_libraries(test_hardware_op_def
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareOpDefTests COMMAND test_hardware_op_def)

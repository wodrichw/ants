add_executable(test_hardware_command_config
    test_command_config.cpp
)

target_link_libraries(test_hardware_command_config
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareCommandConfigTests COMMAND test_hardware_command_config)

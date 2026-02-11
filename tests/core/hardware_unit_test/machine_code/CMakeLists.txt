add_executable(test_hardware_machine_code
    test_machine_code.cpp
)

target_link_libraries(test_hardware_machine_code
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareMachineCodeTests COMMAND test_hardware_machine_code)

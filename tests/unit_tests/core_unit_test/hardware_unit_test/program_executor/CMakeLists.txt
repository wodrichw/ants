add_executable(test_hardware_program_executor
    test_program_executor.cpp
)

target_link_libraries(test_hardware_program_executor
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareProgramExecutorTests COMMAND test_hardware_program_executor)

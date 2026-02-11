add_executable(test_hardware_hardware_manager
    test_hardware_manager.cpp
)

target_link_libraries(test_hardware_hardware_manager
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareManagerTests COMMAND test_hardware_hardware_manager)

add_executable(test_hardware_software_manager
    test_software_manager.cpp
)

target_link_libraries(test_hardware_software_manager
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareSoftwareManagerTests COMMAND test_hardware_software_manager)

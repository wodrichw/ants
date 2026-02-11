add_executable(test_hardware_label_map
    test_label_map.cpp
)

target_link_libraries(test_hardware_label_map
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareLabelMapTests COMMAND test_hardware_label_map)

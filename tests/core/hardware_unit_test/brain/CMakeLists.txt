add_executable(test_hardware_brain
    test_brain.cpp
)

target_link_libraries(test_hardware_brain
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareBrainTests COMMAND test_hardware_brain)

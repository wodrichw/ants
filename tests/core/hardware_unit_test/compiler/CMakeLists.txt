add_executable(test_hardware_compiler
    test_compiler.cpp
)

target_link_libraries(test_hardware_compiler
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareCompilerTests COMMAND test_hardware_compiler)

add_executable(test_hardware_parser
    test_parser.cpp
    test_parser_edge_cases.cpp
)

target_link_libraries(test_hardware_parser
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareParserTests COMMAND test_hardware_parser)

add_executable(test_hardware_token_parser
    test_token_parser.cpp
    test_token_parser_edge_cases.cpp
)

target_link_libraries(test_hardware_token_parser
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME HardwareTokenParserTests COMMAND test_hardware_token_parser)

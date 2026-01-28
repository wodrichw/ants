add_executable(test_proto
    test_proto.cpp
)

target_link_libraries(test_proto
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME ProtoUnitTests COMMAND test_proto)

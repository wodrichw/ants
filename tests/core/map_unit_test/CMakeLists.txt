add_executable(test_map
    test_map.cpp
)

target_link_libraries(test_map
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME MapUnitTests COMMAND test_map)

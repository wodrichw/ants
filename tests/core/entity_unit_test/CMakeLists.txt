add_executable(test_entity
    test_entity.cpp
)

target_link_libraries(test_entity
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME EntityUnitTests COMMAND test_entity)

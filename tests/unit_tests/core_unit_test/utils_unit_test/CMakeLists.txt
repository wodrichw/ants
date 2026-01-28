add_executable(test_utils
    test_utils.cpp
)

target_link_libraries(test_utils
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME UtilsUnitTests COMMAND test_utils)

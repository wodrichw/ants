add_executable(test_app
    test_app.cpp
    test_clock_speed.cpp
)

target_link_libraries(test_app
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME AppUnitTests COMMAND test_app)
add_test(NAME SaveRestoreUnitTests COMMAND test_app --gtest_filter=SaveRestore*)
add_test(NAME ClockSpeedUnitTests COMMAND test_app --gtest_filter=ClockSpeed*)

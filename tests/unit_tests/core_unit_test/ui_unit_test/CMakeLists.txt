add_executable(test_ui
    test_ui.cpp
    test_keyboard.cpp
)

target_link_libraries(test_ui
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pthread
)

add_test(NAME UiUnitTests COMMAND test_ui)

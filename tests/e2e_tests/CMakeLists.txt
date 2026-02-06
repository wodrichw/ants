add_executable(test_e2e
    ${CMAKE_CURRENT_SOURCE_DIR}/action_e2e_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keyboard_e2e_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/replay_e2e_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/save_restore_e2e_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/e2e_helpers.cpp
)

target_link_libraries(test_e2e
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    pthread
    ants_src
)

target_compile_definitions(test_e2e PRIVATE
    E2E_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test_assets"
)

add_test(NAME E2ETests COMMAND test_e2e)
add_test(NAME E2eSaveRestoreTests COMMAND test_e2e --gtest_filter=E2eSaveRestore*)
add_test(NAME E2eKeyboard COMMAND test_e2e --gtest_filter=E2eKeyboard*)

list(APPEND TEST_TARGETS test_e2e)

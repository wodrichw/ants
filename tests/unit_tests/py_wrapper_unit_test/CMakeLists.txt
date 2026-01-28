add_executable(test_py_wrapper
    test_py_wrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/py_wrapper/py_wrapper.cpp
)

target_link_libraries(test_py_wrapper
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ProtoSources
    ants_src
    pybind11::embed
    pthread
)

add_test(NAME PyWrapperUnitTests COMMAND test_py_wrapper)

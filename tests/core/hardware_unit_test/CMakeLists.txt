# Hardware unit tests

# Create executable for program executor tests
add_executable(test_program_executor
    test_program_executor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/hardware/program_executor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/entity/entity_data.cpp
    ${CMAKE_SOURCE_DIR}/src/core/utils/thread_pool.cpp
    # Add other required source files as needed
)

# Link libraries
target_link_libraries(test_program_executor
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
    ${Protobuf_LIBRARIES}
    pthread
)

# Add to tests
add_test(NAME ProgramExecutorTests COMMAND test_program_executor)
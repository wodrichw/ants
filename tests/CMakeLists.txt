cmake_minimum_required(VERSION 3.20)

# Find required packages
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Protobuf REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-linux/include)

# Create test directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_assets/maps)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_assets/saves)

# Add subdirectories for each test module
add_subdirectory(core/hardware_unit_test)
add_subdirectory(core/entity_unit_test)
add_subdirectory(core/map_unit_test)
add_subdirectory(core/serialization_unit_test)
add_subdirectory(core/assembly_unit_test)

# Create a target to run all tests
add_custom_target(run_tests
    COMMAND ctest --output-on-failure
    DEPENDS 
        test_program_executor
        test_entity
        test_map_generation
        test_serialization
        test_assembly_parser
    COMMENT "Running all unit tests"
)

# Enable testing
enable_testing()
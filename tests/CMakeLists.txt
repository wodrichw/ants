cmake_minimum_required(VERSION 3.20)

# Enable testing before registering tests
enable_testing()

# Find required packages
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Protobuf REQUIRED)
find_package(absl CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
if(DEFINED VCPKG_TARGET_TRIPLET)
    include_directories(${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)
else()
    include_directories(${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/include)
endif()

if(WIN32)
    add_library(pthread INTERFACE)
endif()

# Create test directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests/core_unit_test)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_assets/maps)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_assets/saves)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_assets/replays/generated)

# Add subdirectories for each test module
set(TEST_TARGETS)

add_subdirectory(unit_tests/core_unit_test/hardware_unit_test)
list(APPEND TEST_TARGETS
    test_hardware_program_executor
    test_hardware_label_map
    test_hardware_machine_code
    test_hardware_token_parser
    test_hardware_parser
    test_hardware_command_config
    test_hardware_compiler
    test_hardware_op_def
    test_hardware_hardware_manager
    test_hardware_software_manager
    test_hardware_brain
)

add_subdirectory(unit_tests/core_unit_test/app_unit_test)
list(APPEND TEST_TARGETS test_app)

add_subdirectory(unit_tests/core_unit_test/entity_unit_test)
list(APPEND TEST_TARGETS test_entity)

add_subdirectory(unit_tests/core_unit_test/map_unit_test)
list(APPEND TEST_TARGETS test_map)

add_subdirectory(unit_tests/core_unit_test/ui_unit_test)
list(APPEND TEST_TARGETS test_ui)

add_subdirectory(unit_tests/core_unit_test/utils_unit_test)
list(APPEND TEST_TARGETS test_utils)

add_subdirectory(unit_tests/proto_unit_test)
list(APPEND TEST_TARGETS test_proto)

add_subdirectory(unit_tests/py_wrapper_unit_test)
list(APPEND TEST_TARGETS test_py_wrapper)

option(E2E_TESTS "Compile end-to-end tests" OFF)
if (E2E_TESTS)
    add_subdirectory(e2e_tests)
endif()

if(MSVC)
    foreach(test_target IN LISTS TEST_TARGETS)
        target_compile_options(${test_target} PRIVATE /wd4251 /wd4267)
    endforeach()
endif()

# Create a target to run all tests
add_custom_target(run_tests
    COMMAND ctest --output-on-failure
    DEPENDS ${TEST_TARGETS}
    COMMENT "Running all unit tests"
)

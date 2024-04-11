cmake_minimum_required (VERSION 3.13...3.21)

set (CMAKE_EXPORT_COMPILE_COMMANDS true)

# Set the default toolchain to use a Vcpkg submodule.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake")

project (ants C CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake OPTIONAL RESULT_VARIABLE CONAN_FILE)
if(CONAN_FILE)
    conan_basic_setup(TARGETS)
    set(LINK_TCOD CONAN_PKG::libtcod)
    set(SDL2MAIN)
else()
    find_package(SDL2 CONFIG REQUIRED)
    find_package(libtcod CONFIG REQUIRED)
    set(LINK_TCOD libtcod::libtcod)
    set(SDL2MAIN SDL2::SDL2main)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

# This and KEEP_RPATHS is required to handle RPATH's on MacOS.
if (APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

add_custom_target(copy_data_dir
    COMMENT "Copy project data directory to the runtime folder."
    COMMAND cmake -E copy_directory
        ${CMAKE_SOURCE_DIR}/../data
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data
)
add_custom_target(copy_font
    COMMENT "Copy terminal.png to the runtime folder."
    COMMAND cmake -E copy_if_different
        ${CMAKE_SOURCE_DIR}/../terminal.png
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/terminal.png
)

# Enforce UTF-8 encoding on MSVC.
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Enable extra warnings for all samples.
if(MSVC)
    add_compile_options("/W4")
else()
    add_compile_options("-Wall" "-Wextra")
endif()

# Set OPT/DBG/LOG flags based on the cmake variant
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode")
    set(OPT ON)
    set(DBG OFF)
    set(LOG OFF)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode")
    set(OPT OFF)
    set(DBG ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "Building in Release with Debug Info mode")
    set(OPT ON)
    set(DBG ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    message(STATUS "Building in Minimum Size Release mode")
    set(OPT ON)
    set(DBG OFF)
    set(LOG OFF)
endif()

# Check if the user wants to enable optimization flags
option(OPT "Enable optimization flags" OFF)
if(OPT)
    message(STATUS "Enabling optimization flags")
    add_compile_options("-O3")
endif()

# Check if the user wants to enable debug flags
option(DBG "Enable debug flags" OFF)
if(DBG)
    message(STATUS "Enabling debug flags")
    add_compile_options("-g")
endif()

# Set the compile time log level for the project
option(LOG "Set compile time log level" "INFO")
message(STATUS "Setting log level to ${LOG}")
if(LOG STREQUAL "TRACE")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
elseif(LOG STREQUAL "DEBUG")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
elseif(LOG STREQUAL "INFO")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
elseif(LOG STREQUAL "WARN")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN)
elseif(LOG STREQUAL "ERROR")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_ERROR)
elseif(LOG STREQUAL "CRITICAL")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_CRITICAL)
else()
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)
endif()

include_directories(${PROJECT_SOURCE_DIR}/lib/spdlog/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/spdlog)

# pybind11
include_directories(${PROJECT_SOURCE_DIR}/lib/pybind11/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)

# add_executable(samples_c samples_c.c)
# target_link_libraries(samples_c ${LINK_TCOD} ${SDL2MAIN})
if (CONAN_FILE)
    add_dependencies(samples_c copy_data_dir)
endif()
# Suppress MSVC warnings for POSIX C functions such as strdup.  This is only for the C samples.
# target_compile_definitions(samples_c PRIVATE "$<$<C_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>")

add_subdirectory(src)
include_directories(${PROJECT_SOURCE_DIR}/src/core)
add_executable(ants src/main.cpp)
set_target_properties(ants PROPERTIES CXX_STANDARD 17)
target_link_libraries(ants ants_src)
if (CONAN_FILE)
    add_dependencies(ants copy_font)
endif()

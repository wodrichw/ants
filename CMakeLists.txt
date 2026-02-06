cmake_minimum_required(VERSION 3.13)

set (CMAKE_EXPORT_COMPILE_COMMANDS true)

# Set the default toolchain to use a Vcpkg submodule.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake")
set (STD 20)
set(CMAKE_CXX_STANDARD ${STD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ants LANGUAGES C CXX VERSION 0.0.1)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else()
    foreach(flag_var IN ITEMS
        CMAKE_C_FLAGS
        CMAKE_CXX_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_MINSIZEREL
    )
        string(REPLACE "-fPIC" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/Zi" "/Z7" ${flag_var} "${${flag_var}}")
        set(${flag_var} "${${flag_var}}" CACHE STRING "" FORCE)
    endforeach()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FS" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS" CACHE STRING "" FORCE)
endif()
add_executable(ants src/main.cpp)
set_target_properties(ants PROPERTIES CXX_STANDARD ${STD} CXX_STANDARD_REQUIRED
    ON)

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
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/FS>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/FS>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

# Enable extra warnings for all samples.
if(MSVC)
    add_compile_options("/W4")
    add_compile_options("/utf-8")
    add_compile_definitions(FMT_UNICODE=0)
    # Suppress MSVC warnings from protobuf headers.
    add_compile_options("/wd4251" "/wd4267")
else()
    add_compile_options("-Wall" "-Wextra")
endif()

# Set OPT/DBG/LOG flags based on the cmake variant
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode")
    set(OPT ON)
    set(DBG OFF)
    set(LOG OFF)
    set(DBG_GRAPHICS OFF)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode")
    set(OPT OFF)
    set(DBG ON)
    set(DBG_GRAPHICS ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "Building in Release with Debug Info mode")
    set(OPT ON)
    set(DBG ON)
    set(DBG_GRAPHICS OFF)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    message(STATUS "Building in Minimum Size Release mode")
    set(OPT ON)
    set(DBG OFF)
    set(LOG OFF)
    set(DBG_GRAPHICS OFF)
endif()

# Check if the user wants to enable optimization flags
option(OPT "Enable optimization flags" OFF)
if(OPT)
    message(STATUS "Enabling optimization flags")
    if(MSVC)
        add_compile_options("/O2")
    else()
        add_compile_options("-O3")
    endif()
else()
    message(STATUS "Disabling optimization flags")
    if(MSVC)
        add_compile_options("/Od")
    else()
        add_compile_options("-O0")
    endif()
endif()

# Check if the user wants to enable debug flags
option(DBG "Enable debug flags" OFF)
if(DBG)
    message(STATUS "Enabling debug flags")
    if(MSVC)
        add_compile_options("/Z7")
    else()
        add_compile_options("-g")
    endif()
endif()

if (DBG_GRAPHICS)
    message(STATUS "Enabling debug graphics")
    add_compile_options("-DDBG_GRAPHICS")
endif()

# Set the compile time log level for the project
option(LOG "Set compile time log level" "INFO")
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
    set(LOG OFF)
endif()

message(STATUS "Setting log level to ${LOG}")

find_package(spdlog CONFIG REQUIRED)
include_directories(${SPDLOG_INCLUDE_DIRS})

find_package(pybind11 CONFIG REQUIRED)
include_directories(${PYBIND11_INCLUDE_DIRS})

find_package(Protobuf CONFIG REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# add_executable(samples_c samples_c.c)
# target_link_libraries(samples_c ${LINK_TCOD} ${SDL2MAIN})
if (CONAN_FILE)
    add_dependencies(samples_c copy_data_dir)
endif()
# Suppress MSVC warnings for POSIX C functions such as strdup.  This is only for the C samples.
# target_compile_definitions(samples_c PRIVATE "$<$<C_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>")

add_subdirectory(src)
include_directories(${PROJECT_SOURCE_DIR}/src/core ${PROTOBUF_INCLUDE_DIRS_1})
target_link_libraries(ants ants_src)
if(MSVC)
    target_compile_options(ants PRIVATE /wd4251 /wd4267)
endif()
if (CONAN_FILE)
    add_dependencies(ants copy_font)
endif()

if (BENCH)
    message(STATUS "Creating ant benchmarks executable")
    add_executable(bench src/benchmark/ant_bench.cpp)
    target_link_libraries(bench ants_benchmark)
endif()

option(UNIT_TEST "Compile unit tests" OFF)
if (UNIT_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()
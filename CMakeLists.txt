cmake_minimum_required (VERSION 3.13...3.21)

set (CMAKE_EXPORT_COMPILE_COMMANDS true)

# Set the default toolchain to use a Vcpkg submodule.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

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


# Check if the user wants to enable optimization flags  
option(OPT "Enable optimization flags" OFF)  
if(OPT)  
    add_compile_options("-O3")
endif()  
  
# Check if the user wants to enable debug flags  
option(DBG "Enable debug flags" OFF)  
if(DBG)  
    add_compile_options("-g")
endif()  

# add_executable(samples_c samples_c.c)
# target_link_libraries(samples_c ${LINK_TCOD} ${SDL2MAIN})
if (CONAN_FILE)
    add_dependencies(samples_c copy_data_dir)
endif()
# Suppress MSVC warnings for POSIX C functions such as strdup.  This is only for the C samples.
# target_compile_definitions(samples_c PRIVATE "$<$<C_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>")

add_executable(ants
    src/main.cpp
    src/ant.cpp
    src/ant.hpp
    src/map.cpp
    src/map.hpp
    src/controller.cpp
    src/controller.hpp
    src/buttonController.cpp
    src/buttonController.hpp
    src/engine.cpp
    src/engine.hpp
    src/colors.hpp
    src/building.cpp
    src/building.hpp
    src/operations.hpp
    src/operations.cpp
    src/parser.hpp
    src/parser.cpp
    src/text_editor_handler.hpp
    src/text_editor_handler.cpp
    src/render.hpp
    src/render.cpp
    src/arg_parse.hpp
    src/arg_parse.cpp
)
set_target_properties(ants PROPERTIES CXX_STANDARD 17)

target_link_libraries(ants ${LINK_TCOD} ${SDL2MAIN})
if (CONAN_FILE)
    add_dependencies(ants copy_font)
endif()


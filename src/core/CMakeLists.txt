# Collect Source Files
message(STATUS "Collecting source files for ants")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Collect all source and header files in the src/ directory
file(GLOB_RECURSE src_files
    "*.cpp"
    "*.hpp"
)

# Add the sources to a library target
add_library(ants_src ${src_files})
target_link_libraries(ants_src PRIVATE ${LINK_TCOD} ${SDL2MAIN} spdlog::spdlog)

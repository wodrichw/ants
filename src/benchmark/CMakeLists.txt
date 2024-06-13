## Collect Source Files
message(STATUS "Collecting source files for the benchmarking")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Collect all source and header files in the src/ directory
file(GLOB_RECURSE src_benchmark_files
    "*.cpp"
    "*.hpp"
)

# Add the sources to a library target
add_library(ants_benchmark ${src_benchmark_files})

find_package(benchmark CONFIG REQUIRED)
target_link_libraries(ants_benchmark PRIVATE
    benchmark::benchmark
)

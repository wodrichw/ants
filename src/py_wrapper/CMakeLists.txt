message(STATUS "Compiling Python wrapper")

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)  
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../core)
include_directories(${Python3_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS_1})
pybind11_add_module(ant_core py_wrapper.cpp)
target_link_libraries(ant_core PRIVATE ants_src ${Python3_LIBRARIES} ${LINK_TCOD} ${SDL2MAIN} spdlog::spdlog)

# Set the appropriate compiler flags
if(APPLE)
    set_target_properties(ant_core PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
        SUFFIX ".so"  # Force .so extension on macOS
    )
else()
    set_target_properties(ant_core PROPERTIES
        SUFFIX ".so"  # Force .so extension
    )
endif()
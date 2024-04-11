pybind11_add_module(ant_core py_wrapper.cpp)
target_link_libraries(ant_core PRIVATE ants_src ${Python3_LIBRARIES} ${LINK_TCOD} ${SDL2MAIN} spdlog::spdlog)

# Set the appropriate compiler flags
if(APPLE)
    set_target_properties(ant_core PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
        SUFFIX ".so"  # Force .so extension on macOS
    )
endif()
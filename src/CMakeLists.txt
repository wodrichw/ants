add_subdirectory(proto)
add_subdirectory(core)

if (PYWRAP)
    add_subdirectory(py_wrapper)
endif()

if (BENCH)
    add_subdirectory(benchmark)
endif()
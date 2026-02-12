# src/reports/range/formatters/typst/CMakeLists.txt

add_library(RangeTypFormatter SHARED
    range_typ_formatter.cpp
    range_typ_config.cpp
)

setup_plugin_target(RangeTypFormatter NO_STDCXXEXP)
target_link_libraries(RangeTypFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(RangeTypFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )

    target_link_options(RangeTypFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(RangeTypFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: RangeTypFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: RangeTypFormatter with maximum size optimization")
endif()

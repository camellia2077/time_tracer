# src/reports/range/formatters/markdown/CMakeLists.txt

add_library(RangeMdFormatter SHARED
    range_md_formatter.cpp
    range_md_formatter_core.cpp
)

setup_plugin_target(RangeMdFormatter NO_STDCXXEXP)
target_link_libraries(RangeMdFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(RangeMdFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )

    target_link_options(RangeMdFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(RangeMdFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: RangeMdFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: RangeMdFormatter with maximum size optimization")
endif()

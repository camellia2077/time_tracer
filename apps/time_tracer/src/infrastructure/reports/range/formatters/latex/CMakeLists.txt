# src/reports/range/formatters/latex/CMakeLists.txt

add_library(RangeTexFormatter SHARED
    range_tex_formatter.cpp
    range_tex_utils.cpp
)

setup_plugin_target(RangeTexFormatter NO_STDCXXEXP)
target_link_libraries(RangeTexFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(RangeTexFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )

    target_link_options(RangeTexFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(RangeTexFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: RangeTexFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: RangeTexFormatter with maximum size optimization")
endif()

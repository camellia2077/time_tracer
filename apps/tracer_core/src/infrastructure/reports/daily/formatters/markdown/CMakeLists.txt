# src/reports/daily/formatters/markdown/CMakeLists.txt

add_library(DayMdFormatter SHARED
    day_md_formatter.cpp
    day_md_formatter_core.cpp
)

setup_plugin_target(DayMdFormatter NO_STDCXXEXP)
target_link_libraries(DayMdFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(DayMdFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(DayMdFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(DayMdFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: DayMdFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: DayMdFormatter with maximum size optimization")
endif()

# src/reports/monthly/formatters/markdown/CMakeLists.txt

add_library(MonthMdFormatter SHARED
    month_md_formatter.cpp
    month_md_config.cpp
)

setup_plugin_target(MonthMdFormatter NO_STDCXXEXP)
target_link_libraries(MonthMdFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(MonthMdFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(MonthMdFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(MonthMdFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: MonthMdFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: MonthMdFormatter with maximum size optimization")
endif()

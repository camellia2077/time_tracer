# src/reports/monthly/formatters/latex/CMakeLists.txt

add_library(MonthTexFormatter SHARED
    month_tex_formatter.cpp
    month_tex_formatter_core.cpp
    month_tex_utils.cpp
)

setup_plugin_target(MonthTexFormatter NO_STDCXXEXP)
target_link_libraries(MonthTexFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(MonthTexFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(MonthTexFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(MonthTexFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: MonthTexFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: MonthTexFormatter with maximum size optimization")
endif()

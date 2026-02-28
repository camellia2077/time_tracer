# src/reports/daily/formatters/latex/CMakeLists.txt

add_library(DayTexFormatter SHARED
    day_tex_formatter.cpp
    day_tex_formatter_core.cpp
    day_tex_utils.cpp
    ../statistics/latex_strategy.cpp
)

setup_plugin_target(DayTexFormatter NO_STDCXXEXP)
target_link_libraries(DayTexFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(DayTexFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(DayTexFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(DayTexFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: DayTexFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: DayTexFormatter with maximum size optimization")
endif()

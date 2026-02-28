# src/reports/monthly/formatters/typst/CMakeLists.txt

add_library(MonthTypFormatter SHARED
    month_typ_formatter.cpp
    month_typ_formatter_core.cpp
)

setup_plugin_target(MonthTypFormatter NO_STDCXXEXP)
target_link_libraries(MonthTypFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(MonthTypFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(MonthTypFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(MonthTypFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: MonthTypFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: MonthTypFormatter with maximum size optimization")
endif()

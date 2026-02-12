# src/reports/daily/formatters/typst/CMakeLists.txt

add_library(DayTypFormatter SHARED
    day_typ_formatter.cpp
    day_typ_config.cpp
    day_typ_utils.cpp
    ../statistics/typst_strategy.cpp
)

setup_plugin_target(DayTypFormatter NO_STDCXXEXP)
target_link_libraries(DayTypFormatter PRIVATE reports_shared)

if(NOT DISABLE_OPTIMIZATION)
    target_compile_options(DayTypFormatter PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(DayTypFormatter PRIVATE
        -s
        -Wl,--gc-sections
    )
endif()

set_target_properties(DayTypFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

if(DISABLE_OPTIMIZATION)
    message(STATUS "Configured shared library target: DayTypFormatter with size optimization disabled (--no-opt)")
else()
    message(STATUS "Configured shared library target: DayTypFormatter with maximum size optimization")
endif()

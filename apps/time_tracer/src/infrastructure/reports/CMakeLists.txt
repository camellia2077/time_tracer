# src/infrastructure/reports/CMakeLists.txt

# --- Shared Reports Library ---
add_library(reports_shared SHARED
    # Base Configs
    "daily/common/day_base_config.cpp"
    "monthly/common/month_base_config.cpp"
    "range/common/range_base_config.cpp"
    "daily/formatters/statistics/stat_formatter.cpp"

    # Shared - Internal Utils & Config
    "shared/utils/config/config_utils.cpp"
    "shared/utils/format/report_string_utils.cpp"
    "shared/utils/format/time_format.cpp"
    "shared/interfaces/formatter_c_config_bridge_v1.cpp"
    "shared/interfaces/formatter_c_string_view_utils.cpp"
    "shared/interfaces/formatter_c_report_data_bridge_validate.cpp"
    "shared/interfaces/formatter_c_report_data_bridge_parse.cpp"
    
    # Shared - Style Config
    "shared/config/tex_style_config.cpp"
    "shared/config/typst_style_config.cpp"

    # Shared - Formatters
    "shared/formatters/base/project_tree_formatter.cpp"
    "shared/formatters/markdown/markdown_formatter.cpp" 
    "shared/formatters/typst/typ_utils.cpp"
    "shared/formatters/latex/tex_utils.cpp"
    "shared/formatters/latex/tex_common_utils.cpp"
)

target_compile_definitions(reports_shared PRIVATE REPORTS_SHARED_EXPORTS)
set_target_properties(reports_shared PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
setup_plugin_target(reports_shared NO_STDCXXEXP)
target_link_libraries(reports_shared PUBLIC
    nlohmann_json::nlohmann_json
    time_tracker_domain # Needed for time utils maybe?
)


# --- Reports Data Library ---
add_library(reports_data STATIC
    # Utils
    "data/utils/project_tree_builder.cpp"

    # Daily Queriers
    "data/queriers/daily/daily_querier.cpp"

    # Monthly Queriers
    "data/queriers/monthly/monthly_querier.cpp"

    # Period Queriers
    "data/queriers/period/period_querier.cpp"
    "data/queriers/period/batch_period_data_fetcher.cpp"

    # Weekly Queriers
    "data/queriers/weekly/weekly_querier.cpp"

    # Yearly Queriers
    "data/queriers/yearly/yearly_querier.cpp"
)
setup_plugin_target(reports_data NO_STDCXXEXP)
target_link_libraries(reports_data PUBLIC 
    reports_shared 
    SQLite3
    time_tracker_domain
)

# Formatter plugin subdirectories
add_subdirectory(daily/formatters/markdown)
add_subdirectory(daily/formatters/typst)
add_subdirectory(daily/formatters/latex)
add_subdirectory(range/formatters/markdown)
add_subdirectory(range/formatters/typst)
add_subdirectory(range/formatters/latex)
add_subdirectory(monthly/formatters/markdown)
add_subdirectory(monthly/formatters/typst)
add_subdirectory(monthly/formatters/latex)

# src/infrastructure/reports/CMakeLists.txt

# --- Shared Reports Library ---
add_library(reports_shared SHARED
    # Shared - External Utils
    "../../shared/utils/string_utils.cpp"
    
    # Base Configs
    "daily/common/day_base_config.cpp"
    "monthly/common/month_base_config.cpp"
    "range/common/range_base_config.cpp"
    "daily/formatters/statistics/stat_formatter.cpp"

    # Shared - Internal Utils & Config
    "shared/utils/config/config_utils.cpp"
    "shared/utils/format/bool_to_string.cpp"
    "shared/utils/format/report_string_utils.cpp"
    "shared/utils/format/iso_week_utils.cpp"
    "shared/utils/format/year_utils.cpp"
    "shared/utils/format/time_format.cpp"
    "shared/interfaces/formatter_c_config_bridge_v1.cpp"
    "shared/interfaces/formatter_c_string_view_utils.cpp"
    "shared/interfaces/formatter_c_report_data_bridge.cpp"
    
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
    "data/queriers/daily/day_querier.cpp"
    "data/queriers/daily/batch_day_data_fetcher.cpp"

    # Monthly Queriers
    "data/queriers/monthly/month_querier.cpp"
    "data/queriers/monthly/batch_month_data_fetcher.cpp"

    # Period Queriers
    "data/queriers/period/period_querier.cpp"
    "data/queriers/period/batch_period_data_fetcher.cpp"

    # Weekly Queriers
    "data/queriers/weekly/week_querier.cpp"
    "data/queriers/weekly/batch_week_data_fetcher.cpp"

    # Yearly Queriers
    "data/queriers/yearly/year_querier.cpp"
    "data/queriers/yearly/batch_year_data_fetcher.cpp"
)
setup_plugin_target(reports_data NO_STDCXXEXP)
target_link_libraries(reports_data PUBLIC 
    reports_shared 
    SQLite3
    time_tracker_domain
)

# Export targets to parent scope if needed, or just link by name "reports_shared"

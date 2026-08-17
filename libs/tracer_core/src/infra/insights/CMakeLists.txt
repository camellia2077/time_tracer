# src/infra/insights/CMakeLists.txt

# --- Shared Insights Library ---
add_library(insights_shared SHARED
    # Base Configs
    "daily/common/daily_formatter_base_config.cpp"
    "monthly/common/month_base_config.cpp"
    "range/common/range_base_config.cpp"
    "daily/formatters/statistics/stat_formatter.cpp"

    # Shared - Internal Utils & Config
    "shared/utils/format/insights_string_utils.cpp"
    "shared/utils/format/time_format.cpp"
    
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

target_compile_definitions(insights_shared PRIVATE INSIGHTS_SHARED_EXPORTS)
set_target_properties(insights_shared PROPERTIES
    PREFIX ""
    OUTPUT_NAME "insights_shared"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
setup_app_target(insights_shared NO_STDCXXEXP)
target_link_libraries(insights_shared PUBLIC
    nlohmann_json::nlohmann_json
    time_tracker_domain # Needed for time utils maybe?
)


# --- Insights Data Library ---
add_library(insights_data STATIC
    # Utils
    "data/utils/project_tree_builder.cpp"

    # Daily Queriers
    "data/queriers/daily/daily_querier.cpp"

    # Monthly Queriers
    "data/queriers/monthly/monthly_querier.cpp"

    # Period Queriers
    "data/queriers/period/period_querier.cpp"
    "data/queriers/period/batch_period_data_fetcher.cpp"
    "data/queriers/range/date_range_querier.cpp"

    # Weekly Queriers
    "data/queriers/weekly/weekly_querier.cpp"

    # Yearly Queriers
    "data/queriers/yearly/yearly_querier.cpp"
)
setup_plugin_target(
    insights_data
    NO_STDCXXEXP
    PCH_HEADER "${PROJECT_SOURCE_DIR}/pch_sqlite.hpp"
)
target_link_libraries(insights_data PUBLIC 
    insights_shared 
    SQLite3
    time_tracker_domain
)

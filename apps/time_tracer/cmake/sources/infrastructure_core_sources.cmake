set(TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE "")
set(TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE "")
if(ANDROID)
    set(TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE
        "platform/android/android_platform_clock.cpp")
elseif(WIN32)
    set(TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE
        "platform/windows/windows_platform_clock.cpp")
    set(TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE
        "platform/android/android_platform_clock.cpp")
else()
    message(STATUS
        "Non-Windows desktop build detected, defaulting to windows_platform_clock.cpp.")
    set(TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE
        "platform/windows/windows_platform_clock.cpp")
    set(TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE
        "platform/android/android_platform_clock.cpp")
endif()

set(TIME_TRACKER_INFRA_CORE_SOURCES
    "query/data/data_query_repository.cpp"
    "query/data/data_query_sql_builders.cpp"
    "query/data/data_query_row_mappers.cpp"
    "query/data/data_query_repository_sql.cpp"
    "query/data/stats/day_duration_stats_calculator.cpp"
    "query/data/stats/report_chart_stats_calculator.cpp"
    "query/data/stats/stats_boundary.cpp"
    "query/data/orchestrators/date_range_resolver.cpp"
    "query/data/orchestrators/list_query_orchestrator.cpp"
    "query/data/orchestrators/days_stats_orchestrator.cpp"
    "query/data/orchestrators/report_chart_orchestrator.cpp"
    "query/data/orchestrators/tree_orchestrator.cpp"
    "query/data/orchestrators/orchestrators_boundary.cpp"
    "query/data/renderers/data_query_renderer.cpp"
    "query/data/renderers/text_renderer.cpp"
    "query/data/renderers/semantic_json_renderer.cpp"
    "query/data/renderers/renderers_boundary.cpp"
    "${TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE}"
    "${TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE}"
    "logging/console_logger.cpp"
    "logging/console_diagnostics_sink.cpp"
    "logging/file_error_report_writer.cpp"
    "logging/validation_issue_reporter.cpp"
)

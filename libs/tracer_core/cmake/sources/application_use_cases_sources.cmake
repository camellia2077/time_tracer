set(TIME_TRACKER_APPLICATION_USE_CASE_FAMILY_SOURCES
    "use_cases/core_api_failure.cpp"
    "use_cases/report_api_support.cpp"
    "use_cases/pipeline_api.cpp"
    "use_cases/query_api.cpp"
    "use_cases/report_api.cpp"
    "use_cases/tracer_exchange_api.cpp"
    "aggregate_runtime/tracer_core_runtime.cpp"
)

set(TIME_TRACKER_APPLICATION_USE_CASE_SOURCES
    ${TIME_TRACKER_APPLICATION_USE_CASE_FAMILY_SOURCES}
    "runtime_bridge/logger.cpp"
)

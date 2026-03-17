set(TIME_TRACKER_APPLICATION_USE_CASE_FAMILY_SOURCES
    "use_cases/tracer_core_api_helpers.module.cpp"
    "use_cases/tracer_core_api_pipeline.module.cpp"
    "use_cases/tracer_core_api_report.module.cpp"
    "use_cases/tracer_core_api_query.module.cpp"
)

set(TIME_TRACKER_APPLICATION_USE_CASE_SOURCES
    ${TIME_TRACKER_APPLICATION_USE_CASE_FAMILY_SOURCES}
    "ports/logger.cpp"
)

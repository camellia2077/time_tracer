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
    "bootstrap/android_runtime_factory.cpp"
    "query/data/data_query_repository.cpp"
    "query/data/data_query_repository_sql.cpp"
    "query/data/data_query_statistics.cpp"
    "query/data/data_query_output.cpp"
    "io/processed_data_io.cpp"
    "io/file_io_service.cpp"
    "io/txt_ingest_input_provider.cpp"
    "${TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE}"
    "${TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE}"
    "io/core/file_reader.cpp"
    "io/core/file_writer.cpp"
    "io/core/file_system_helper.cpp"
    "io/utils/file_utils.cpp"
    "logging/console_logger.cpp"
    "logging/console_diagnostics_sink.cpp"
    "logging/file_error_report_writer.cpp"
    "serialization/json_serializer.cpp"
    "serialization/core/log_codec.cpp"
)

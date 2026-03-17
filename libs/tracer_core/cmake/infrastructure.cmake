include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/infrastructure_core_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/infrastructure_logging_sources.cmake")

set(TRACER_CORE_INFRA_LITE_SOURCES
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/config/static_converter_config_provider.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/${TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE}"
)
list(APPEND TRACER_CORE_INFRA_LITE_SOURCES
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/config/static_converter_config_provider.module.cpp"
)
list(TRANSFORM TIME_TRACKER_INFRA_LOGGING_SOURCES
    PREPEND "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/"
)
list(PREPEND TRACER_CORE_INFRA_LITE_SOURCES
    ${TIME_TRACKER_INFRA_LOGGING_SOURCES}
)

if(TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE)
    list(APPEND TRACER_CORE_INFRA_LITE_SOURCES
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/${TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE}"
    )
endif()

add_library(tc_infra_lite_lib STATIC)
target_sources(tc_infra_lite_lib PRIVATE
    ${TRACER_CORE_INFRA_LITE_SOURCES}
)

target_sources(tc_infra_lite_lib PUBLIC
    FILE_SET core_infrastructure_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules"
    FILES
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/logging/tracer.core.infrastructure.logging.console_logger.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/logging/tracer.core.infrastructure.logging.console_diagnostics_sink.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/logging/tracer.core.infrastructure.logging.file_error_report_writer.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/logging/tracer.core.infrastructure.logging.validation_issue_reporter.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/logging/tracer.core.infrastructure.logging.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/platform/tc.core.infra.plat.windows.clock.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/platform/tc.core.infra.plat.android.clock.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/config/tracer.core.infrastructure.config.static_converter_config_provider.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tracer.core.infrastructure.cppm"
)
set_target_properties(tc_infra_lite_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

if(COMMAND setup_app_target)
    setup_app_target(tc_infra_lite_lib NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
endif()

target_include_directories(tc_infra_lite_lib PUBLIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_lite_lib PUBLIC
    tc_app_lib
)
target_link_libraries(tc_infra_lite_lib PRIVATE
    nlohmann_json::nlohmann_json
)

add_library(tc_all_lib INTERFACE)
target_link_libraries(tc_all_lib INTERFACE
    tc_shared_lib
    tc_domain_lib
    tc_app_lib
    tc_infra_lite_lib
)

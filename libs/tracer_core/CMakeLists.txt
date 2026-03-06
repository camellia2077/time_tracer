set(TRACER_CORE_LIB_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TRACER_CORE_LIB_CMAKE_SOURCES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sources")

add_library(tc_shared_lib STATIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/utils/period_utils.cpp"
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(tc_shared_lib PUBLIC
        FILE_SET core_shared_modules TYPE CXX_MODULES
        BASE_DIRS
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules"
        FILES
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.string_utils.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.period_utils.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.exceptions.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.exit_codes.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.ansi_colors.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.cppm"
    )
    set_target_properties(tc_shared_lib PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

if(COMMAND setup_plugin_target)
    setup_plugin_target(tc_shared_lib NO_STDCXXEXP NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
elseif(COMMAND setup_app_target)
    setup_app_target(tc_shared_lib NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
endif()

target_include_directories(tc_shared_lib PUBLIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}"
)

set(TRACER_CORE_DOMAIN_SOURCES
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/utils/time_utils.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/ports/diagnostics.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/converter/convert/core/converter_core.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/converter/convert/core/converter_core_stats.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/converter/convert/core/converter_core_activity_mapper.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/validator/json/rules/json_rules.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/validator/txt/facade/text_validator.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/validator/txt/rules/txt_rules.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/validator/structure/structure_validator.cpp"
)

add_library(tc_domain_lib STATIC)
target_sources(tc_domain_lib PRIVATE
    ${TRACER_CORE_DOMAIN_SOURCES}
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(tc_domain_lib PUBLIC
        FILE_SET core_domain_modules TYPE CXX_MODULES
        BASE_DIRS
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules"
        FILES
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.types.date_check_mode.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.types.ingest_mode.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.types.converter_config.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.types.app_options.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.errors.error_record.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.ports.diagnostics.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.repos.project_repo.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.model.source_span.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.model.time_data_models.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.model.processing_result.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.model.daily_log.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.rpt.types.report_types.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.rpt.models.project_tree.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.rpt.models.range_report_data.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.rpt.models.period_report_models.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.rpt.models.daily_report_data.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.rpt.models.query_data_structs.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.logic.converter.core.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.logic.conv.log_processor.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.logic.valid.common.diag.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.logic.valid.common.valid_utils.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.logic.validator.txt.rules.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tc.core.dom.logic.valid.txt.facade.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.logic.validator.structure.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.logic.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/modules/tracer.core.domain.cppm"
    )
    set_target_properties(tc_domain_lib PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

if(COMMAND setup_plugin_target)
    setup_plugin_target(tc_domain_lib NO_STDCXXEXP NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
elseif(COMMAND setup_app_target)
    setup_app_target(tc_domain_lib NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
endif()

target_include_directories(tc_domain_lib PUBLIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}"
)
target_link_libraries(tc_domain_lib PUBLIC
    tc_shared_lib
)
target_link_libraries(tc_domain_lib PRIVATE
    nlohmann_json::nlohmann_json
)

include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_bootstrap_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_use_cases_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_pipeline_sources.cmake")

set(TRACER_CORE_APPLICATION_SOURCES
    ${TIME_TRACKER_APPLICATION_BOOTSTRAP_SOURCES}
    ${TIME_TRACKER_APPLICATION_USE_CASE_SOURCES}
    ${TIME_TRACKER_APPLICATION_PIPELINE_SOURCES}
)
list(TRANSFORM TRACER_CORE_APPLICATION_SOURCES
    PREPEND "${TRACER_CORE_LIB_SOURCE_ROOT}/application/"
)

add_library(tc_app_lib STATIC)
target_sources(tc_app_lib PRIVATE
    ${TRACER_CORE_APPLICATION_SOURCES}
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(tc_app_lib PUBLIC
        FILE_SET core_application_modules TYPE CXX_MODULES
        BASE_DIRS
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules"
        FILES
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.use_cases.interface.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.use_cases.api.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.service.converter.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.importer.service.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.context.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.stages.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.manager.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.workflow_handler.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.cppm"
    )
    set_target_properties(tc_app_lib PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

if(COMMAND setup_app_target)
    setup_app_target(
        tc_app_lib
        NO_PCH
        NO_APP_SOURCE_ROOT_INCLUDE
    )
endif()

target_include_directories(tc_app_lib PUBLIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}"
)
target_link_libraries(tc_app_lib PUBLIC
    tc_domain_lib
)

include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/infrastructure_core_sources.cmake")

set(TRACER_CORE_INFRA_LITE_SOURCES
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/logging/console_logger.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/logging/console_diagnostics_sink.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/logging/file_error_report_writer.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/logging/validation_issue_reporter.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/config/static_converter_config_provider.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/${TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE}"
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

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(tc_infra_lite_lib PUBLIC
        FILE_SET core_infrastructure_modules TYPE CXX_MODULES
        BASE_DIRS
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules"
        FILES
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.log.console_logger.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.log.console_diag_sink.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.log.file_error_report_writer.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.log.valid_issue_reporter.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.plat.windows.clock.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.plat.android.clock.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tc.core.infra.cfg.static_conv_cfg_prov.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/modules/tracer.core.infrastructure.cppm"
    )
    set_target_properties(tc_infra_lite_lib PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

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




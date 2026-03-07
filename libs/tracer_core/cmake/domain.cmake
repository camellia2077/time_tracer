set(TRACER_CORE_DOMAIN_SOURCES
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/utils/time_utils.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/ports/diagnostics.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/converter/convert/core/converter_core.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/converter/convert/core/converter_core_stats.cpp"
    "${TRACER_CORE_LIB_SOURCE_ROOT}/domain/logic/converter/convert/core/converter_core_activity_mapper.cpp"
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

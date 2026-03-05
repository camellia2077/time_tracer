# src/domain/CMakeLists.txt
add_library(time_tracker_domain STATIC
    "utils/time_utils.cpp"
    "ports/diagnostics.cpp"
    "../shared/utils/period_utils.cpp"
    "logic/converter/convert/core/converter_core.cpp"
    "logic/converter/convert/core/converter_core_stats.cpp"
    "logic/converter/convert/core/converter_core_activity_mapper.cpp"
    "logic/validator/json/rules/json_rules.cpp"
    "logic/validator/txt/facade/text_validator.cpp"
    "logic/validator/txt/rules/txt_rules.cpp"
    "logic/validator/structure/structure_validator.cpp"
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(time_tracker_domain PUBLIC
        FILE_SET core_shared_modules TYPE CXX_MODULES
        BASE_DIRS
            "${PROJECT_SOURCE_DIR}/src/shared/modules"
        FILES
            "${PROJECT_SOURCE_DIR}/src/shared/modules/tracer.core.shared.string_utils.cppm"
            "${PROJECT_SOURCE_DIR}/src/shared/modules/tracer.core.shared.period_utils.cppm"
            "${PROJECT_SOURCE_DIR}/src/shared/modules/tracer.core.shared.exceptions.cppm"
            "${PROJECT_SOURCE_DIR}/src/shared/modules/tracer.core.shared.exit_codes.cppm"
            "${PROJECT_SOURCE_DIR}/src/shared/modules/tracer.core.shared.ansi_colors.cppm"
            "${PROJECT_SOURCE_DIR}/src/shared/modules/tracer.core.shared.cppm"
    )
    target_sources(time_tracker_domain PUBLIC
        FILE_SET core_domain_modules TYPE CXX_MODULES
        BASE_DIRS
            "${PROJECT_SOURCE_DIR}/src/domain/modules"
        FILES
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.types.date_check_mode.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.types.ingest_mode.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.types.converter_config.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.types.app_options.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.errors.error_record.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.ports.diagnostics.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.repositories.project_repository.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.model.source_span.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.model.time_data_models.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.model.processing_result.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.model.daily_log.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.reports.types.report_types.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.reports.models.project_tree.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.reports.models.range_report_data.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.reports.models.period_report_models.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.reports.models.daily_report_data.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.reports.models.query_data_structs.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.converter.core.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.converter.log_processor.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.validator.common.diagnostic.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.validator.common.validator_utils.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.validator.txt.rules.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.validator.txt.facade.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.validator.structure.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.logic.cppm"
            "${PROJECT_SOURCE_DIR}/src/domain/modules/tracer.core.domain.cppm"
    )
    set_target_properties(time_tracker_domain PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

setup_plugin_target(time_tracker_domain NO_STDCXXEXP NO_PCH)
target_link_libraries(time_tracker_domain PRIVATE
    nlohmann_json::nlohmann_json
)

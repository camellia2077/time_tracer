#
# Infrastructure assembly owned here:
# - reports shared/data/full report adapters
# - persistence/config/full infrastructure targets
# This directory expects the parent `libs/tracer_core` layer targets to
# already exist, especially `tc_infra_lite_lib` and `tc_all_lib`.
#

set(TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/sources"
)
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_core_sources.cmake")
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_persistence_sources.cmake")
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_reports_sources.cmake")
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_config_sources.cmake")

if(NOT TARGET taio_lib)
    message(FATAL_ERROR "Required target missing: taio_lib")
endif()
if(NOT TARGET tc_infra_lite_lib)
    message(FATAL_ERROR
        "Required target missing: tc_infra_lite_lib")
endif()

set(TRACER_CORE_INFRA_LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../src/infrastructure")
set(TRACER_CORE_REPORTS_ROOT "${TRACER_CORE_INFRA_LIB_ROOT}/reports")

set(TRACER_CORE_REPORTS_SHARED_SOURCES
    "${TRACER_CORE_REPORTS_ROOT}/daily/common/day_base_config.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/monthly/common/month_base_config.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/range/common/range_base_config.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/daily/formatters/statistics/stat_formatter.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/utils/format/report_string_utils.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/utils/format/time_format.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/config/tex_style_config.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/config/typst_style_config.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/formatters/base/project_tree_formatter.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/formatters/markdown/markdown_formatter.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/formatters/typst/typ_utils.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/formatters/latex/tex_utils.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/shared/formatters/latex/tex_common_utils.cpp"
)

add_library(tc_rpt_shared_lib SHARED)
target_sources(tc_rpt_shared_lib PRIVATE
    ${TRACER_CORE_REPORTS_SHARED_SOURCES}
)
target_compile_definitions(tc_rpt_shared_lib PRIVATE REPORTS_SHARED_EXPORTS)
set_target_properties(tc_rpt_shared_lib PROPERTIES
    PREFIX ""
    OUTPUT_NAME "reports_shared"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
setup_app_target(tc_rpt_shared_lib NO_STDCXXEXP NO_APP_SOURCE_ROOT_INCLUDE)
target_link_libraries(tc_rpt_shared_lib PUBLIC
    nlohmann_json::nlohmann_json
    tc_domain_lib
)

set(TRACER_CORE_REPORTS_DATA_SOURCES
    "${TRACER_CORE_REPORTS_ROOT}/data/utils/project_tree_builder.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/daily/daily_querier.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/monthly/monthly_querier.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/period/period_querier.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/period/batch_period_data_fetcher.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/range/date_range_querier.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/weekly/weekly_querier.cpp"
    "${TRACER_CORE_REPORTS_ROOT}/data/queriers/yearly/yearly_querier.cpp"
)

add_library(tc_rpt_data_lib STATIC)
target_sources(tc_rpt_data_lib PRIVATE
    ${TRACER_CORE_REPORTS_DATA_SOURCES}
)
setup_plugin_target(
    tc_rpt_data_lib
    NO_STDCXXEXP
    PCH_HEADER "${PROJECT_SOURCE_DIR}/pch_sqlite.hpp"
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_link_libraries(tc_rpt_data_lib PUBLIC
    tc_rpt_shared_lib
    SQLite3
    tc_domain_lib
)

set(TRACER_CORE_INFRA_REPORTS_SOURCES ${TIME_TRACKER_INFRA_REPORTS_SOURCES})
list(FILTER TRACER_CORE_INFRA_REPORTS_SOURCES EXCLUDE REGEX "^$")
list(TRANSFORM TRACER_CORE_INFRA_REPORTS_SOURCES
    PREPEND "${TRACER_CORE_INFRA_LIB_ROOT}/"
)

add_library(tc_infra_reports_lib STATIC)
target_sources(tc_infra_reports_lib PRIVATE
    ${TRACER_CORE_INFRA_REPORTS_SOURCES}
)
target_sources(tc_infra_reports_lib PUBLIC
    FILE_SET core_infrastructure_reports_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules"
    FILES
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/exporting/tracer.core.infrastructure.reports.exporting.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/exporting/tracer.core.infrastructure.reports.exporting.export_utils.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/exporting/tracer.core.infrastructure.reports.exporting.report_file_manager.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/exporting/tracer.core.infrastructure.reports.exporting.exporter.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/dto/tracer.core.infrastructure.reports.dto.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/dto/tracer.core.infrastructure.reports.dto.formatter.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/dto/tracer.core.infrastructure.reports.dto.export_writer.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/data_querying/tracer.core.infrastructure.reports.data_querying.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/data_querying/tracer.core.infrastructure.reports.data_querying.lazy_sqlite_report_data_query_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/data_querying/tracer.core.infrastructure.reports.data_querying.sqlite_report_data_query_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/tracer.core.infrastructure.reports.querying.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/tracer.core.infrastructure.reports.querying.lazy_sqlite_report_query_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/tracer.core.infrastructure.reports.querying.report_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/services/tracer.core.infrastructure.reports.querying.services.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/services/tracer.core.infrastructure.reports.querying.services.daily_report_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/services/tracer.core.infrastructure.reports.querying.services.monthly_report_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/services/tracer.core.infrastructure.reports.querying.services.weekly_report_service.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/reports/querying/services/tracer.core.infrastructure.reports.querying.services.yearly_report_service.cppm"
)
set_target_properties(tc_infra_reports_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_reports_lib
    PCH_HEADER "${PROJECT_SOURCE_DIR}/pch_sqlite_toml.hpp"
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_link_libraries(tc_infra_reports_lib PUBLIC
    taio_lib
    tc_all_lib
    tc_rpt_data_lib
    tc_rpt_shared_lib
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)

set(TRACER_CORE_INFRA_CORE_SOURCES_REMAINING ${TIME_TRACKER_INFRA_CORE_SOURCES})
list(REMOVE_ITEM TRACER_CORE_INFRA_CORE_SOURCES_REMAINING
    "config/static_converter_config_provider.cpp"
    "${TIME_TRACKER_INFRA_PLATFORM_CLOCK_SOURCE}"
)
if(TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE)
    list(REMOVE_ITEM TRACER_CORE_INFRA_CORE_SOURCES_REMAINING
        "${TIME_TRACKER_INFRA_ANDROID_HELPER_CLOCK_SOURCE}"
    )
endif()
list(FILTER TRACER_CORE_INFRA_CORE_SOURCES_REMAINING EXCLUDE REGEX "^$")

set(TRACER_CORE_INFRA_PERSISTENCE_SOURCES ${TIME_TRACKER_INFRA_PERSISTENCE_SOURCES})
set(TRACER_CORE_INFRA_CONFIG_SOURCES ${TIME_TRACKER_INFRA_CONFIG_SOURCES})

list(TRANSFORM TRACER_CORE_INFRA_CORE_SOURCES_REMAINING
    PREPEND "${TRACER_CORE_INFRA_LIB_ROOT}/"
)
list(TRANSFORM TRACER_CORE_INFRA_PERSISTENCE_SOURCES
    PREPEND "${TRACER_CORE_INFRA_LIB_ROOT}/"
)
list(TRANSFORM TRACER_CORE_INFRA_CONFIG_SOURCES
    PREPEND "${TRACER_CORE_INFRA_LIB_ROOT}/"
)

add_library(tc_infra_full_lib STATIC)
target_sources(tc_infra_full_lib PRIVATE
    ${TRACER_CORE_INFRA_CORE_SOURCES_REMAINING}
    ${TRACER_CORE_INFRA_PERSISTENCE_SOURCES}
    ${TRACER_CORE_INFRA_CONFIG_SOURCES}
)
target_sources(tc_infra_full_lib PUBLIC
    FILE_SET core_infrastructure_full_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules"
    FILES
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/tracer.core.infrastructure.config.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/loader/tracer.core.infrastructure.config.loader.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/internal/tracer.core.infrastructure.config.internal.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/tracer.core.infrastructure.config.config_loader.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/tracer.core.infrastructure.config.file_converter_config_provider.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/loader/tracer.core.infrastructure.config.loader.toml_loader_utils.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/loader/tracer.core.infrastructure.config.loader.report_config_loader.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/loader/tracer.core.infrastructure.config.loader.converter_config_loader.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/internal/tracer.core.infrastructure.config.internal.config_detail_loader.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/internal/tracer.core.infrastructure.config.internal.android_bundle_config_paths.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/internal/tracer.core.infrastructure.config.internal.cli_config_snapshot.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/config/internal/tracer.core.infrastructure.config.internal.config_parser_utils.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/repository/tracer.core.infrastructure.query.data.repository.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/repository/tracer.core.infrastructure.query.data.repository.models.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/repository/tracer.core.infrastructure.query.data.repository.types.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/repository/tracer.core.infrastructure.query.data.repository.data_query_repository.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/internal/tracer.core.infrastructure.query.data.internal.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/internal/tracer.core.infrastructure.query.data.internal.request.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/internal/tracer.core.infrastructure.query.data.internal.period.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/internal/tracer.core.infrastructure.query.data.internal.report_mapping.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/renderers/tracer.core.infrastructure.query.data.renderers.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/renderers/tracer.core.infrastructure.query.data.renderers.data_query_renderer.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/renderers/tracer.core.infrastructure.query.data.renderers.text_renderer.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/renderers/tracer.core.infrastructure.query.data.renderers.semantic_json_renderer.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/renderers/tracer.core.infrastructure.query.data.renderers.boundary.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.list_query_orchestrator.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.days_stats_orchestrator.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.report_chart_orchestrator.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.tree_orchestrator.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.date_range_resolver.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/orchestrators/tracer.core.infrastructure.query.data.orchestrators.boundary.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/stats/tracer.core.infrastructure.query.data.stats.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/stats/tracer.core.infrastructure.query.data.stats.models.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/stats/tracer.core.infrastructure.query.data.stats.day_duration_stats_calculator.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/stats/tracer.core.infrastructure.query.data.stats.report_chart_stats_calculator.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/query/data/stats/tracer.core.infrastructure.query.data.stats.boundary.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/tracer.core.infrastructure.persistence.write.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/runtime/tracer.core.infrastructure.persistence.runtime.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/tracer.core.infrastructure.persistence.write.importer.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/sqlite/tracer.core.infrastructure.persistence.write.importer.sqlite.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/tracer.core.infrastructure.persistence.write.sqlite_time_sheet_repository.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/runtime/tracer.core.infrastructure.persistence.runtime.sqlite_project_repository.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/runtime/tracer.core.infrastructure.persistence.runtime.sqlite_database_health_checker.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/tracer.core.infrastructure.persistence.write.importer.repository.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/sqlite/tracer.core.infrastructure.persistence.write.importer.sqlite.connection.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/sqlite/tracer.core.infrastructure.persistence.write.importer.sqlite.statement.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/sqlite/tracer.core.infrastructure.persistence.write.importer.sqlite.writer.cppm"
        "${TRACER_CORE_INFRA_LIB_ROOT}/modules/persistence/write/importer/sqlite/tracer.core.infrastructure.persistence.write.importer.sqlite.project_resolver.cppm"
)
set_target_properties(tc_infra_full_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

setup_app_target(
    tc_infra_full_lib
    PCH_HEADER "${PROJECT_SOURCE_DIR}/pch_sqlite_toml.hpp"
    NO_APP_SOURCE_ROOT_INCLUDE
)

target_link_libraries(tc_infra_full_lib PUBLIC
    taio_lib
    tc_infra_reports_lib
    tc_infra_lite_lib
    tc_all_lib
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)
if(NOT TARGET sodium)
    message(FATAL_ERROR "Required target missing: sodium")
endif()
if(NOT TARGET zstd::libzstd)
    message(FATAL_ERROR "Required target missing: zstd::libzstd")
endif()
target_link_libraries(tc_infra_full_lib PUBLIC
    sodium
    zstd::libzstd
)

add_library(tc_adapters_iface INTERFACE)
target_link_libraries(tc_adapters_iface INTERFACE
    tc_infra_full_lib
)




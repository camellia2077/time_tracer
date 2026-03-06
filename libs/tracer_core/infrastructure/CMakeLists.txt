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
    "${TRACER_CORE_REPORTS_ROOT}/shared/utils/config/config_utils.cpp"
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
    "logging/console_logger.cpp"
    "logging/console_diagnostics_sink.cpp"
    "logging/file_error_report_writer.cpp"
    "logging/validation_issue_reporter.cpp"
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




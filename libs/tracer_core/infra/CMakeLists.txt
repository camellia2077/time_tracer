#
# Infrastructure assembly owned here:
# - reporting shared/data/full reporting adapters
# - capability-aligned query/config/exchange/persistence targets
# - compatibility aggregation for tracer_core.dll consumers
#

set(TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/sources"
)
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_core_sources.cmake")
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_query_sources.cmake")
include("${TRACER_CORE_INFRA_LIB_CMAKE_SOURCES_ROOT}/infrastructure_exchange_sources.cmake")
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
if(NOT TARGET tc_all_lib)
    message(FATAL_ERROR "Required target missing: tc_all_lib")
endif()
if(NOT TARGET sodium)
    message(FATAL_ERROR "Required target missing: sodium")
endif()
if(NOT TARGET zstd::libzstd)
    message(FATAL_ERROR "Required target missing: zstd::libzstd")
endif()

set(TRACER_CORE_INFRA_LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../src/infra")
set(TRACER_CORE_INFRA_MODULE_ROOT "${TRACER_CORE_INFRA_LIB_ROOT}/modules")
set(TRACER_CORE_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(TRACER_CORE_REPORTS_ROOT "${TRACER_CORE_INFRA_LIB_ROOT}/reporting")

function(tc_prepend_infra_root out_var)
    set(_files)
    foreach(_relative IN LISTS ARGN)
        list(APPEND _files "${TRACER_CORE_INFRA_LIB_ROOT}/${_relative}")
    endforeach()
    set(${out_var} "${_files}" PARENT_SCOPE)
endfunction()

function(tc_prepend_module_root out_var)
    set(_files)
    foreach(_relative IN LISTS ARGN)
        list(APPEND _files "${TRACER_CORE_INFRA_MODULE_ROOT}/${_relative}")
    endforeach()
    set(${out_var} "${_files}" PARENT_SCOPE)
endfunction()

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

tc_prepend_infra_root(
    TRACER_CORE_INFRA_PERSISTENCE_RUNTIME_SOURCES_ABS
    ${TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_ALL_SOURCES}
)
tc_prepend_module_root(
    TRACER_CORE_INFRA_PERSISTENCE_RUNTIME_MODULE_FILES_ABS
    ${TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_MODULE_FILES}
)
tc_prepend_infra_root(
    TRACER_CORE_INFRA_PERSISTENCE_WRITE_SOURCES_ABS
    ${TIME_TRACKER_INFRA_PERSISTENCE_WRITE_SOURCES}
)
tc_prepend_module_root(
    TRACER_CORE_INFRA_PERSISTENCE_WRITE_MODULE_FILES_ABS
    ${TIME_TRACKER_INFRA_PERSISTENCE_WRITE_MODULE_FILES}
)
tc_prepend_infra_root(
    TRACER_CORE_INFRA_CONFIG_SOURCES_ABS
    ${TIME_TRACKER_INFRA_CONFIG_SOURCES}
)
tc_prepend_module_root(
    TRACER_CORE_INFRA_CONFIG_MODULE_FILES_ABS
    "config/tracer.core.infrastructure.config.cppm"
    "config/tracer.core.infrastructure.config.config_loader.cppm"
    "config/tracer.core.infrastructure.config.file_converter_config_provider.cppm"
    "config/loader/tracer.core.infrastructure.config.loader.cppm"
    "config/loader/tracer.core.infrastructure.config.loader.converter_config_loader.cppm"
    "config/loader/tracer.core.infrastructure.config.loader.report_config_loader.cppm"
    "config/loader/tracer.core.infrastructure.config.loader.toml_loader_utils.cppm"
    "config/internal/tracer.core.infrastructure.config.internal.cppm"
    "config/internal/tracer.core.infrastructure.config.internal.android_bundle_config_paths.cppm"
    "config/internal/tracer.core.infrastructure.config.internal.cli_config_snapshot.cppm"
    "config/internal/tracer.core.infrastructure.config.internal.config_detail_loader.cppm"
    "config/internal/tracer.core.infrastructure.config.internal.config_parser_utils.cppm"
)
tc_prepend_infra_root(
    TRACER_CORE_INFRA_QUERY_SOURCES_ABS
    ${TIME_TRACKER_INFRA_QUERY_SOURCES}
)
tc_prepend_module_root(
    TRACER_CORE_INFRA_QUERY_MODULE_FILES_ABS
    ${TIME_TRACKER_INFRA_QUERY_MODULE_FILES}
)
tc_prepend_infra_root(
    TRACER_CORE_INFRA_REPORTING_SOURCES_ABS
    ${TIME_TRACKER_INFRA_REPORTING_SOURCES}
)
tc_prepend_module_root(
    TRACER_CORE_INFRA_REPORTING_MODULE_FILES_ABS
    ${TIME_TRACKER_INFRA_REPORTING_MODULE_FILES}
)
tc_prepend_infra_root(
    TRACER_CORE_INFRA_EXCHANGE_SOURCES_ABS
    ${TIME_TRACKER_INFRA_EXCHANGE_SOURCES}
)
tc_prepend_module_root(
    TRACER_CORE_INFRA_EXCHANGE_MODULE_FILES_ABS
    ${TIME_TRACKER_INFRA_EXCHANGE_MODULE_FILES}
)

add_library(tc_infra_persistence_runtime_lib STATIC)
target_sources(tc_infra_persistence_runtime_lib PRIVATE
    ${TRACER_CORE_INFRA_PERSISTENCE_RUNTIME_SOURCES_ABS}
)
target_sources(tc_infra_persistence_runtime_lib PUBLIC
    FILE_SET core_infrastructure_persistence_runtime_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_MODULE_ROOT}"
    FILES
        ${TRACER_CORE_INFRA_PERSISTENCE_RUNTIME_MODULE_FILES_ABS}
)
set_target_properties(tc_infra_persistence_runtime_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_persistence_runtime_lib
    NO_PCH
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_include_directories(tc_infra_persistence_runtime_lib PUBLIC
    "${TRACER_CORE_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_persistence_runtime_lib PUBLIC
    tc_all_lib
    SQLite3
)

add_library(tc_infra_persistence_write_lib STATIC)
target_sources(tc_infra_persistence_write_lib PRIVATE
    ${TRACER_CORE_INFRA_PERSISTENCE_WRITE_SOURCES_ABS}
)
target_sources(tc_infra_persistence_write_lib PUBLIC
    FILE_SET core_infrastructure_persistence_write_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_MODULE_ROOT}"
    FILES
        ${TRACER_CORE_INFRA_PERSISTENCE_WRITE_MODULE_FILES_ABS}
)
set_target_properties(tc_infra_persistence_write_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_persistence_write_lib
    NO_PCH
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_include_directories(tc_infra_persistence_write_lib PUBLIC
    "${TRACER_CORE_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_persistence_write_lib PUBLIC
    tc_all_lib
    tc_infra_persistence_runtime_lib
    SQLite3
)

add_library(tc_infra_config_lib STATIC)
target_sources(tc_infra_config_lib PRIVATE
    ${TRACER_CORE_INFRA_CONFIG_SOURCES_ABS}
)
target_sources(tc_infra_config_lib PUBLIC
    FILE_SET core_infrastructure_config_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_MODULE_ROOT}"
    FILES
        ${TRACER_CORE_INFRA_CONFIG_MODULE_FILES_ABS}
)
set_target_properties(tc_infra_config_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_config_lib
    NO_PCH
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_include_directories(tc_infra_config_lib PUBLIC
    "${TRACER_CORE_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_config_lib PUBLIC
    tc_all_lib
    tomlplusplus::tomlplusplus
)

add_library(tc_infra_query_lib STATIC)
target_sources(tc_infra_query_lib PRIVATE
    ${TRACER_CORE_INFRA_QUERY_SOURCES_ABS}
)
target_sources(tc_infra_query_lib PUBLIC
    FILE_SET core_infrastructure_query_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_MODULE_ROOT}"
    FILES
        ${TRACER_CORE_INFRA_QUERY_MODULE_FILES_ABS}
)
set_target_properties(tc_infra_query_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_query_lib
    NO_PCH
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_include_directories(tc_infra_query_lib PUBLIC
    "${TRACER_CORE_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_query_lib PUBLIC
    tc_all_lib
    tc_infra_config_lib
    tc_infra_persistence_runtime_lib
    SQLite3
    nlohmann_json::nlohmann_json
)

add_library(tc_infra_reporting_lib STATIC)
target_sources(tc_infra_reporting_lib PRIVATE
    ${TRACER_CORE_INFRA_REPORTING_SOURCES_ABS}
)
target_sources(tc_infra_reporting_lib PUBLIC
    FILE_SET core_infrastructure_reporting_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_MODULE_ROOT}"
    FILES
        ${TRACER_CORE_INFRA_REPORTING_MODULE_FILES_ABS}
)
set_target_properties(tc_infra_reporting_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_reporting_lib
    NO_PCH
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_include_directories(tc_infra_reporting_lib PUBLIC
    "${TRACER_CORE_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_reporting_lib PUBLIC
    tc_all_lib
    tc_infra_config_lib
    tc_infra_persistence_runtime_lib
    tc_rpt_data_lib
    tc_rpt_shared_lib
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)

add_library(tc_infra_exchange_lib STATIC)
target_sources(tc_infra_exchange_lib PRIVATE
    ${TRACER_CORE_INFRA_EXCHANGE_SOURCES_ABS}
)
target_sources(tc_infra_exchange_lib PUBLIC
    FILE_SET core_infrastructure_exchange_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_INFRA_MODULE_ROOT}"
    FILES
        ${TRACER_CORE_INFRA_EXCHANGE_MODULE_FILES_ABS}
)
set_target_properties(tc_infra_exchange_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)
setup_app_target(
    tc_infra_exchange_lib
    NO_PCH
    NO_APP_SOURCE_ROOT_INCLUDE
)
target_include_directories(tc_infra_exchange_lib PUBLIC
    "${TRACER_CORE_SOURCE_ROOT}"
)
target_link_libraries(tc_infra_exchange_lib PUBLIC
    tc_all_lib
    tc_infra_config_lib
    sodium
    zstd::libzstd
)

add_library(tc_infra_reports_lib ALIAS tc_infra_reporting_lib)

add_library(tc_cap_config_lib INTERFACE)
target_link_libraries(tc_cap_config_lib INTERFACE
    tc_all_lib
    tc_infra_config_lib
)

add_library(tc_cap_pipeline_lib INTERFACE)
target_link_libraries(tc_cap_pipeline_lib INTERFACE
    tc_all_lib
    tc_cap_config_lib
    tc_infra_persistence_write_lib
    taio_lib
)

add_library(tc_cap_query_lib INTERFACE)
target_link_libraries(tc_cap_query_lib INTERFACE
    tc_all_lib
    tc_cap_config_lib
    tc_infra_persistence_runtime_lib
    tc_infra_query_lib
)

add_library(tc_cap_reporting_lib INTERFACE)
target_link_libraries(tc_cap_reporting_lib INTERFACE
    tc_all_lib
    tc_cap_config_lib
    tc_infra_persistence_runtime_lib
    tc_infra_reporting_lib
)

add_library(tc_cap_exchange_lib INTERFACE)
target_link_libraries(tc_cap_exchange_lib INTERFACE
    tc_all_lib
    tc_cap_config_lib
    tc_infra_exchange_lib
)

add_library(tc_infra_full_lib INTERFACE)
target_link_libraries(tc_infra_full_lib INTERFACE
    tc_cap_pipeline_lib
    tc_cap_query_lib
    tc_cap_reporting_lib
    tc_cap_exchange_lib
)

add_library(tc_adapters_iface INTERFACE)
target_link_libraries(tc_adapters_iface INTERFACE
    tc_infra_full_lib
)

# src/infrastructure/CMakeLists.txt
add_subdirectory(reports)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../modules/tracer_transport"
    "${CMAKE_BINARY_DIR}/modules/tracer_transport"
)

set(TRACER_ADAPTERS_IO_SOURCE_ROOT
    "${PROJECT_SOURCE_DIR}/../../modules/tracer_adapters_io/src"
)

include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_core_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_io_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_persistence_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_reports_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_config_sources.cmake")

add_library(time_tracker_io_adapters STATIC)
target_sources(time_tracker_io_adapters PRIVATE
    ${TIME_TRACKER_INFRA_IO_SOURCES}
)
setup_app_target(time_tracker_io_adapters)
target_include_directories(time_tracker_io_adapters PUBLIC
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}"
)
target_link_libraries(time_tracker_io_adapters PUBLIC
    tracer_transport
    tracer_core
)

if(TT_ENABLE_PROCESSED_JSON_IO)
    target_link_libraries(time_tracker_io_adapters PUBLIC
        nlohmann_json::nlohmann_json
    )
endif()

add_library(ttri STATIC)
target_sources(ttri PRIVATE
    ${TIME_TRACKER_INFRA_REPORTS_SOURCES}
)
setup_app_target(ttri)
target_link_libraries(ttri PUBLIC
    time_tracker_io_adapters
    tracer_core
    reports_data
    reports_shared
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)
# ttri = time_tracker_reports_infrastructure
add_library(time_tracker_reports_infrastructure ALIAS ttri)

add_library(tti STATIC)
target_sources(tti PRIVATE
    ${TIME_TRACKER_INFRA_CORE_SOURCES}
    ${TIME_TRACKER_INFRA_PERSISTENCE_SOURCES}
    ${TIME_TRACKER_INFRA_CONFIG_SOURCES}
)

setup_app_target(tti)

target_link_libraries(tti PUBLIC
    time_tracker_io_adapters
    time_tracker_reports_infrastructure
    tracer_core
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)
# tti = time_tracker_infrastructure
add_library(time_tracker_infrastructure ALIAS tti)

# Phase 1 layering target name for adapter implementations.
add_library(tracer_adapters INTERFACE)
target_link_libraries(tracer_adapters INTERFACE
    tti
)

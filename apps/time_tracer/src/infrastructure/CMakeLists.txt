# src/infrastructure/CMakeLists.txt
add_subdirectory(reports)

include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_core_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_persistence_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_reports_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_config_sources.cmake")

add_library(time_tracker_reports_infrastructure STATIC)
target_sources(time_tracker_reports_infrastructure PRIVATE
    ${TIME_TRACKER_INFRA_REPORTS_SOURCES}
)
setup_app_target(time_tracker_reports_infrastructure)
target_link_libraries(time_tracker_reports_infrastructure PUBLIC
    time_tracker_application
    time_tracker_domain
    reports_data
    reports_shared
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)

add_library(time_tracker_infrastructure STATIC)
target_sources(time_tracker_infrastructure PRIVATE
    ${TIME_TRACKER_INFRA_CORE_SOURCES}
    ${TIME_TRACKER_INFRA_PERSISTENCE_SOURCES}
    ${TIME_TRACKER_INFRA_CONFIG_SOURCES}
)

setup_app_target(time_tracker_infrastructure)

target_link_libraries(time_tracker_infrastructure PUBLIC
    time_tracker_reports_infrastructure
    time_tracker_application
    time_tracker_domain
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)

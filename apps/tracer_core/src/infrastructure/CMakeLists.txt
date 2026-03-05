# src/infrastructure/CMakeLists.txt
add_subdirectory(reports)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_transport"
    "${CMAKE_BINARY_DIR}/libs/tracer_transport"
)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_adapters_io"
    "${CMAKE_BINARY_DIR}/libs/tracer_adapters_io"
)

include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_core_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_persistence_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_reports_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/infrastructure_config_sources.cmake")

if(NOT TARGET tracer_adapters_io)
    message(FATAL_ERROR "Required target missing: tracer_adapters_io")
endif()

add_library(ttri STATIC)
target_sources(ttri PRIVATE
    ${TIME_TRACKER_INFRA_REPORTS_SOURCES}
)
setup_app_target(
    ttri
    PCH_HEADER "${PROJECT_SOURCE_DIR}/src/pch_sqlite_toml.hpp"
)
target_link_libraries(ttri PUBLIC
    tracer_adapters_io
    tracer_core
    reports_data
    reports_shared
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)

add_library(tti STATIC)
target_sources(tti PRIVATE
    ${TIME_TRACKER_INFRA_CORE_SOURCES}
    ${TIME_TRACKER_INFRA_PERSISTENCE_SOURCES}
    ${TIME_TRACKER_INFRA_CONFIG_SOURCES}
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(tti PUBLIC
        FILE_SET core_infrastructure_modules TYPE CXX_MODULES
        BASE_DIRS
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules"
        FILES
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.logging.console_logger.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.logging.console_diagnostics_sink.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.logging.file_error_report_writer.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.logging.validation_issue_reporter.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.platform.windows.clock.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.platform.android.clock.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.config.static_converter_config_provider.cppm"
            "${PROJECT_SOURCE_DIR}/src/infrastructure/modules/tracer.core.infrastructure.cppm"
    )
    set_target_properties(tti PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

setup_app_target(
    tti
    PCH_HEADER "${PROJECT_SOURCE_DIR}/src/pch_sqlite_toml.hpp"
)

target_link_libraries(tti PUBLIC
    tracer_adapters_io
    ttri
    tracer_core
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
target_link_libraries(tti PUBLIC
    sodium
    zstd::libzstd
)
# tti = time_tracker_infrastructure
add_library(time_tracker_infrastructure ALIAS tti)

# Phase 1 layering target name for adapter implementations.
add_library(tracer_adapters INTERFACE)
target_link_libraries(tracer_adapters INTERFACE
    tti
)

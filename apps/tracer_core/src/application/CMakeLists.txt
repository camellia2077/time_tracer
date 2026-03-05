# src/application/CMakeLists.txt
include("${PROJECT_SOURCE_DIR}/cmake/sources/application_bootstrap_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/application_use_cases_sources.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sources/application_pipeline_sources.cmake")

add_library(time_tracker_application STATIC)
target_sources(time_tracker_application PRIVATE
    ${TIME_TRACKER_APPLICATION_BOOTSTRAP_SOURCES}
    ${TIME_TRACKER_APPLICATION_USE_CASE_SOURCES}
    ${TIME_TRACKER_APPLICATION_PIPELINE_SOURCES}
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(time_tracker_application PUBLIC
        FILE_SET core_application_modules TYPE CXX_MODULES
        BASE_DIRS
            "${PROJECT_SOURCE_DIR}/src/application/modules"
        FILES
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.use_cases.interface.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.use_cases.api.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.service.converter.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.importer.service.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.pipeline.context.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.pipeline.stages.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.pipeline.manager.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.workflow_handler.cppm"
            "${PROJECT_SOURCE_DIR}/src/application/modules/tracer.core.application.cppm"
    )
    set_target_properties(time_tracker_application PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

setup_app_target(time_tracker_application NO_PCH)

target_link_libraries(time_tracker_application PUBLIC
    time_tracker_domain
)

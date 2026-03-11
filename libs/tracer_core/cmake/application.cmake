include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_bootstrap_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_reporting_tree_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_use_cases_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_workflow_sources.cmake")
include("${TRACER_CORE_LIB_CMAKE_SOURCES_ROOT}/application_pipeline_sources.cmake")

set(TRACER_CORE_APPLICATION_SOURCES
    ${TIME_TRACKER_APPLICATION_BOOTSTRAP_SOURCES}
    ${TIME_TRACKER_APPLICATION_REPORTING_TREE_SOURCES}
    ${TIME_TRACKER_APPLICATION_USE_CASE_SOURCES}
    ${TIME_TRACKER_APPLICATION_WORKFLOW_SOURCES}
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
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.use_cases.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.use_cases.interface.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.use_cases.api.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.use_cases.helpers.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.reporting.tree.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.reporting.tree.data.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.reporting.tree.nodes.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.reporting.tree.viewer.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.service.converter.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.importer.service.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.types.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.stages.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.orchestrator.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.pipeline.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.workflow.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.workflow.interface.cppm"
            "${TRACER_CORE_LIB_SOURCE_ROOT}/application/modules/tracer.core.application.workflow.handler.cppm"
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

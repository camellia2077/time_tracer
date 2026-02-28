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

setup_app_target(time_tracker_application NO_PCH)

target_link_libraries(time_tracker_application PUBLIC
    time_tracker_domain
)

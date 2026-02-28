set(TRACER_CORE_C_API_SOURCES
    "tracer_core_c_api.cpp"
    "tracer_core_c_api_internal.cpp"
    "tracer_core_c_api_workflow.cpp"
    "tracer_core_c_api_reporting.cpp"
    "../android/android_runtime_factory.cpp"
    "../android/android_runtime_factory_resolver.cpp"
    "../android/android_runtime_factory_catalog.cpp"
)

add_library(tracer_core_shared SHARED
    ${TRACER_CORE_C_API_SOURCES}
)
tt_attach_build_metadata(tracer_core_shared
    PREFIX TRACER_CORE
    HEADER_NAME "tracer_core_build_metadata.hpp"
)
setup_app_target(tracer_core_shared NO_PCH)
target_compile_definitions(tracer_core_shared PRIVATE
    TRACER_CORE_EXPORTS
)
target_include_directories(tracer_core_shared PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(tracer_core_shared PUBLIC
    tracer_adapters
    tracer_core
)
set_target_properties(tracer_core_shared PROPERTIES
    OUTPUT_NAME "tracer_core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
if(WIN32)
    set_target_properties(tracer_core_shared PROPERTIES PREFIX "")
endif()

if(BUILD_TESTING)
    add_executable(tracer_core_c_api_smoke_tests
        "tracer_core_c_api_smoke_tests.cpp"
    )
    setup_app_target(tracer_core_c_api_smoke_tests NO_PCH)
    target_include_directories(tracer_core_c_api_smoke_tests PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
    )
    if(NOT WIN32)
        target_link_libraries(tracer_core_c_api_smoke_tests PRIVATE dl)
    endif()
    add_dependencies(tracer_core_c_api_smoke_tests tracer_core_shared)
    add_test(
        NAME tracer_core_c_api_smoke_tests
        COMMAND $<TARGET_FILE:tracer_core_c_api_smoke_tests>
    )
    set_tests_properties(tracer_core_c_api_smoke_tests PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:tracer_core_c_api_smoke_tests>"
    )

    add_executable(tracer_core_c_api_stability_tests
        "tracer_core_c_api_runtime_tests.cpp"
        "tracer_core_c_api_query_tests.cpp"
        "tracer_core_c_api_error_tests.cpp"
    )
    setup_app_target(tracer_core_c_api_stability_tests NO_PCH)
    target_include_directories(tracer_core_c_api_stability_tests PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
    )
    if(NOT WIN32)
        target_link_libraries(tracer_core_c_api_stability_tests PRIVATE dl)
    endif()
    add_dependencies(tracer_core_c_api_stability_tests tracer_core_shared)
    add_test(
        NAME tracer_core_c_api_stability_tests
        COMMAND $<TARGET_FILE:tracer_core_c_api_stability_tests>
    )
    set_tests_properties(tracer_core_c_api_stability_tests PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:tracer_core_c_api_stability_tests>"
    )
endif()

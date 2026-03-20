set(TRACER_CORE_C_API_SOURCES
    "c_api_parse_bridge.cpp"
    "cli_runtime_config_bridge.cpp"
    "tracer_core_c_api.cpp"
    "tracer_core_c_api_internal.cpp"
    "tracer_core_c_api_runtime_config.cpp"
    "tracer_core_c_api_workflow.cpp"
    "tracer_core_c_api_reporting.cpp"
    "tracer_core_c_api_crypto.cpp"
    "${PROJECT_SOURCE_DIR}/host/crypto_progress_bridge.cpp"
    "${PROJECT_SOURCE_DIR}/host/tracer_exchange_inspect_formatter.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_config_bridge.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory_resolver.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory_catalog.cpp"
)

set_source_files_properties(
    "cli_runtime_config_bridge.cpp"
    "tracer_core_c_api_internal.cpp"
    "tracer_core_c_api_runtime_config.cpp"
    "tracer_core_c_api_workflow.cpp"
    "tracer_core_c_api_reporting.cpp"
    "tracer_core_c_api_crypto.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_config_bridge.cpp"
    "${PROJECT_SOURCE_DIR}/host/android_runtime_factory.cpp"
    PROPERTIES
        CXX_SCAN_FOR_MODULES ON
)

add_library(tc_shared_dll SHARED
    ${TRACER_CORE_C_API_SOURCES}
)
tt_attach_build_metadata(tc_shared_dll
    PREFIX TRACER_CORE
    HEADER_NAME "tracer_core_build_metadata.hpp"
)
setup_app_target(tc_shared_dll NO_PCH)
target_compile_definitions(tc_shared_dll PRIVATE
    TRACER_CORE_EXPORTS
)
target_include_directories(tc_shared_dll PUBLIC
    "${PROJECT_SOURCE_DIR}"
)
target_link_libraries(tc_shared_dll PUBLIC
    tc_adapters_iface
    tc_all_lib
)
set_target_properties(tc_shared_dll PROPERTIES
    OUTPUT_NAME "tracer_core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)
if(WIN32)
    set_target_properties(tc_shared_dll PROPERTIES PREFIX "")
endif()

if(BUILD_TESTING)
    set(TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT
        "${PROJECT_SOURCE_DIR}/tests/integration"
    )

    add_executable(tc_c_api_smoke_tests
        "${TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT}/tracer_core_c_api_smoke_tests.cpp"
    )
    setup_app_target(tc_c_api_smoke_tests NO_PCH)
    target_include_directories(tc_c_api_smoke_tests PRIVATE
        "${PROJECT_SOURCE_DIR}"
    )
    target_link_libraries(tc_c_api_smoke_tests PRIVATE
        nlohmann_json::nlohmann_json
    )
    if(NOT WIN32)
        target_link_libraries(tc_c_api_smoke_tests PRIVATE dl)
    endif()
    add_dependencies(tc_c_api_smoke_tests tc_shared_dll)
    add_test(
        NAME tc_c_api_smoke_tests
        COMMAND $<TARGET_FILE:tc_c_api_smoke_tests>
    )
    set_tests_properties(tc_c_api_smoke_tests PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:tc_c_api_smoke_tests>"
    )

    add_executable(tc_c_api_stability_tests
        "${TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT}/tracer_core_c_api_runtime_support.cpp"
        "${TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT}/tracer_core_c_api_runtime_stress_tests.cpp"
        "${TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT}/tracer_core_c_api_runtime_tests.cpp"
        "${TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT}/tracer_core_c_api_query_tests.cpp"
        "${TRACER_CORE_SHELL_INTEGRATION_TEST_ROOT}/tracer_core_c_api_error_tests.cpp"
    )
    setup_app_target(tc_c_api_stability_tests NO_PCH)
    target_include_directories(tc_c_api_stability_tests PRIVATE
        "${PROJECT_SOURCE_DIR}"
    )
    target_link_libraries(tc_c_api_stability_tests PRIVATE
        nlohmann_json::nlohmann_json
    )
    if(NOT WIN32)
        target_link_libraries(tc_c_api_stability_tests PRIVATE dl)
    endif()
    add_dependencies(tc_c_api_stability_tests tc_shared_dll)
    add_test(
        NAME tc_c_api_stability_tests
        COMMAND $<TARGET_FILE:tc_c_api_stability_tests>
    )
    set_tests_properties(tc_c_api_stability_tests PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:tc_c_api_stability_tests>"
    )
endif()

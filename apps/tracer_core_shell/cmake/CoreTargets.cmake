# src/CMakeLists.txt (Coordinator)

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_core"
    "${CMAKE_BINARY_DIR}/libs/tracer_core"
)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_core_bridge_common"
    "${CMAKE_BINARY_DIR}/libs/tracer_core_bridge_common"
)

# 1. Domain (Pure Logic)
if(NOT TARGET tc_domain_lib)
    message(FATAL_ERROR "Required target missing: tc_domain_lib")
endif()

add_library(time_tracker_domain INTERFACE)
target_link_libraries(time_tracker_domain INTERFACE
    tc_domain_lib
)

# 2. Application (Business Logic, Pipeline)
if(NOT TARGET tc_app_lib)
    message(FATAL_ERROR "Required target missing: tc_app_lib")
endif()

add_library(time_tracker_application INTERFACE)
target_link_libraries(time_tracker_application INTERFACE
    tc_app_lib
)

# Core boundary for cross-platform reuse:
# tc_core_iface := domain + application
add_library(tc_core_iface INTERFACE)
target_link_libraries(tc_core_iface INTERFACE
    time_tracker_domain
    time_tracker_application
)

# Optional AI extension stack insertion point:
# - tracer_core_ai attaches after core semantic layers
# - tracer_ai_provider attaches after tracer_core_ai
include("${PROJECT_SOURCE_DIR}/cmake/AiModuleSlots.cmake")
tt_configure_optional_ai_modules()

set(TRACER_CORE_LIB_SOURCE_ROOT "${PROJECT_SOURCE_DIR}/../../libs/tracer_core/src")
set(TRACER_CORE_SHELL_SOURCE_ROOT "${PROJECT_SOURCE_DIR}")
set(TRACER_CORE_SHELL_HOST_ROOT "${TRACER_CORE_SHELL_SOURCE_ROOT}/host")
set(TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT
    "${TRACER_CORE_SHELL_SOURCE_ROOT}/tests/platform"
)

set_source_files_properties(
    "${TRACER_CORE_SHELL_SOURCE_ROOT}/api/c_api/cli_runtime_config_bridge.cpp"
    "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_config_bridge.cpp"
    "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory.cpp"
    PROPERTIES
        CXX_SCAN_FOR_MODULES ON
)

include("${PROJECT_SOURCE_DIR}/cmake/CoreBoundaryRules.cmake")
enforce_core_include_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    CORE_DIRS
        "domain"
        "application"
    FORBIDDEN_PREFIXES
        "api/"
        "infrastructure/"
        "host/"
        "tests/"
    FORBIDDEN_PATTERNS
        "apps[\\/]tracer_core[\\/]src[\\/](api|infrastructure)[\\/]"
        "libs[\\/]tracer_adapters_io[\\/]"
        "tracer_adapters_io[\\/]src[\\/]"
        "nlohmann[\\/]json(_fwd)?\\.hpp"
)
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "domain"
        "application"
    FORBIDDEN_PATTERNS
        "nlohmann::json"
)

if(EXISTS "${TT_TRACER_CORE_AI_SOURCE_DIR}/src")
    enforce_core_include_boundary(
        ROOT "${TT_TRACER_CORE_AI_SOURCE_DIR}/src"
        CORE_DIRS
            "domain"
            "application"
        FORBIDDEN_PATTERNS
            "nlohmann[\\/]json(_fwd)?\\.hpp"
    )
    enforce_source_content_boundary(
        ROOT "${TT_TRACER_CORE_AI_SOURCE_DIR}/src"
        TARGET_DIRS
            "domain"
            "application"
        FORBIDDEN_PATTERNS
            "nlohmann::json"
    )
endif()

enforce_app_shell_source_boundary(
    ROOT "${PROJECT_SOURCE_DIR}"
    ALLOWED_PREFIXES
        "api/"
        "build/"
        "build_"
        "host/"
        "tests/"
        "cmake/"
    ALLOWED_FILES
        "CMakeLists.txt"
        "README.md"
        "agent.md"
        "pch.hpp"
        "pch_sqlite.hpp"
        "pch_sqlite_toml.hpp"
)

enforce_shell_api_include_boundary(
    ROOT "${TRACER_CORE_SHELL_SOURCE_ROOT}"
    API_DIRS
        "api/android_jni"
        "api/c_api"
    EXEMPT_FILES
        "api/c_api/tracer_core_c_api.cpp"
        "api/c_api/tracer_core_c_api_crypto.cpp"
)

# 3. Infrastructure composition
# - `tracer_transport` / `tracer_adapters_io` stay as standalone adapter libs.
# - `libs/tracer_core/infrastructure` owns reports/persistence/config assembly.
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_transport"
    "${CMAKE_BINARY_DIR}/libs/tracer_transport"
)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_adapters_io"
    "${CMAKE_BINARY_DIR}/libs/tracer_adapters_io"
)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_core/infrastructure"
    "${CMAKE_BINARY_DIR}/libs/tracer_core/infrastructure"
)

if(NOT TARGET tc_infra_full_lib)
    message(FATAL_ERROR "Required target missing: tc_infra_full_lib")
endif()
if(NOT TARGET tc_adapters_iface)
    message(FATAL_ERROR "Required target missing: tc_adapters_iface")
endif()

enforce_core_target_link_boundary(
    CORE_TARGETS
        time_tracker_domain
        time_tracker_application
        tc_core_iface
    FORBIDDEN_TARGETS
        taio_lib
        tc_adapters_iface
        tc_infra_full_lib
        tc_infra_reports_lib
)

# 4. API Layer (Interface)
if(ANDROID)
    message(STATUS
        "Android build detected: JNI bridge target is owned by "
        "apps/android/runtime."
    )
elseif(WIN32)
    message(STATUS
        "apps/tracer_core_shell no longer builds the Windows CLI. "
        "Use apps/tracer_cli/windows for desktop executable delivery."
    )
endif()

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/api/c_api"
    "${CMAKE_BINARY_DIR}/apps/tracer_core_shell/api/c_api"
)

set(TRACER_CORE_LIB_TESTS_ROOT "${PROJECT_SOURCE_DIR}/../../libs/tracer_core/tests")

if(BUILD_TESTING)
    # Long-lived regression surface: module smoke + runtime/contract tests.
    add_executable(tc_shared_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/shared/tests/shared_modules_smoke_tests.cpp"
        )
        setup_app_target(tc_shared_mod_smoke_tests NO_PCH)
        target_link_libraries(tc_shared_mod_smoke_tests PRIVATE
            time_tracker_domain
        )
        add_test(
            NAME tc_shared_mod_smoke_tests
            COMMAND tc_shared_mod_smoke_tests
        )

        add_executable(tc_domain_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/domain/tests/domain_modules_smoke_tests.cpp"
        )
        setup_app_target(tc_domain_mod_smoke_tests NO_PCH)
        target_link_libraries(tc_domain_mod_smoke_tests PRIVATE
            time_tracker_domain
        )
        add_test(
            NAME tc_domain_mod_smoke_tests
            COMMAND tc_domain_mod_smoke_tests
        )

        add_executable(tc_dom_logic_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/domain/tests/domain_logic_modules_smoke_tests.cpp"
        )
        setup_app_target(tc_dom_logic_mod_smoke_tests NO_PCH)
        target_link_libraries(tc_dom_logic_mod_smoke_tests
            PRIVATE
            time_tracker_domain
        )
        add_test(
            NAME tc_dom_logic_mod_smoke_tests
            COMMAND tc_dom_logic_mod_smoke_tests
        )

        add_executable(tc_app_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/application_modules_smoke_tests.cpp"
        )
        setup_app_target(tc_app_mod_smoke_tests NO_PCH)
        target_link_libraries(
            tc_app_mod_smoke_tests PRIVATE
            time_tracker_application
        )
        add_test(
            NAME tc_app_mod_smoke_tests
            COMMAND tc_app_mod_smoke_tests
        )

        set(TC_INFRA_MOD_SMOKE_TEST_SOURCES
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/infrastructure_modules_smoke_tests.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/modules_smoke/logging_platform_config.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/modules_smoke/crypto_exchange.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/modules_smoke/query_stats_repository.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/modules_smoke/query_internal_orchestrators.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/modules_smoke/persistence.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/modules_smoke/reports.cpp"
        )
        add_executable(tc_infra_mod_smoke_tests
            ${TC_INFRA_MOD_SMOKE_TEST_SOURCES}
        )
        setup_app_target(tc_infra_mod_smoke_tests NO_PCH)
        target_include_directories(tc_infra_mod_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(
            tc_infra_mod_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_infra_mod_smoke_tests
            COMMAND tc_infra_mod_smoke_tests
        )

    set(TIME_TRACKER_CORE_API_TEST_SOURCES
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/fakes.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/test_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/convert_ingest_validate_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/report_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/data_query_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/import_service_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/tracer_exchange_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/test_main.cpp"
    )
    add_executable(tt_core_api_tests
        ${TIME_TRACKER_CORE_API_TEST_SOURCES}
    )
    setup_app_target(tt_core_api_tests)
    target_include_directories(tt_core_api_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_core_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME tt_core_api_tests
        COMMAND tt_core_api_tests
    )

    add_executable(tt_android_runtime_tests
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_bootstrap_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_config_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_io_chart_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_io_report_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_io_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_core_config_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_history_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_ingest_guard_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_data_layer_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_structure_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_bundle_policy_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_compat_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_validate_logic_structure_reporting_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/validation_issue_reporter_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/txt_month_header_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_test_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_period_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_stats_scenario_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_tree_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_stats_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_main.cpp"
    )
    if(NOT ANDROID)
        target_sources(tt_android_runtime_tests PRIVATE
            "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_config_bridge.cpp"
            "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory.cpp"
            "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory_resolver.cpp"
            "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory_catalog.cpp"
        )
    endif()
    setup_app_target(tt_android_runtime_tests)
    target_include_directories(tt_android_runtime_tests PRIVATE
        "${TRACER_CORE_SHELL_SOURCE_ROOT}"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}"
    )
    target_link_libraries(tt_android_runtime_tests PRIVATE
        tc_infra_full_lib
    )
    add_test(
        NAME tt_android_runtime_tests
        COMMAND tt_android_runtime_tests
    )

    add_executable(tt_fmt_parity_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/report_formatter/report_formatter_parity_fixture_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/report_formatter/report_formatter_parity_md_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/report_formatter/report_formatter_parity_snapshot_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/report_formatter/report_formatter_parity_tex_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infrastructure/tests/report_formatter/report_formatter_parity_typ_tests.cpp"
    )
    setup_app_target(tt_fmt_parity_tests)
    target_include_directories(tt_fmt_parity_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_fmt_parity_tests PRIVATE
        tc_infra_full_lib
    )
    add_test(
        NAME tt_fmt_parity_tests
        COMMAND tt_fmt_parity_tests
    )

    add_executable(tt_file_crypto_tests
        "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_config_bridge.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory_resolver.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/android_runtime_factory_catalog.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_roundtrip_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_failure_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_progress_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_interop_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_package_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_export_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_import_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_test_main.cpp"
    )
    setup_app_target(tt_file_crypto_tests)
    target_include_directories(tt_file_crypto_tests PRIVATE
        "${TRACER_CORE_SHELL_SOURCE_ROOT}"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}"
    )
    target_link_libraries(tt_file_crypto_tests PRIVATE
        tc_infra_full_lib
    )
    add_test(
        NAME tt_file_crypto_tests
        COMMAND tt_file_crypto_tests
    )
endif()




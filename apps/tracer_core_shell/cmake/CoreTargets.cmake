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

set(TT_FORBIDDEN_LEGACY_BOUNDARY_INCLUDE_PATTERNS
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/dto/core_requests\\.hpp"
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/dto/core_responses\\.hpp"
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/dto/tree_query_response\\.hpp"
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/interfaces/i_report_"
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/use_cases/i_tracer_core_runtime\\.hpp"
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/use_cases/tracer_core_runtime\\.hpp"
)
set(TT_FORBIDDEN_AGGREGATE_INCLUDE_PATTERNS
    ${TT_FORBIDDEN_LEGACY_BOUNDARY_INCLUDE_PATTERNS}
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/dto/compat/"
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/aggregate_runtime/"
)
set(TT_FORBIDDEN_NON_OWNER_INCLUDE_PATTERNS
    ${TT_FORBIDDEN_AGGREGATE_INCLUDE_PATTERNS}
    "^[ \t]*#[ \t]*include[ \t]*[<\"]application/compat/"
)

set_source_files_properties(
    "${TRACER_CORE_SHELL_SOURCE_ROOT}/api/c_api/capabilities/config/cli_runtime_config_bridge.cpp"
    "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_config_bridge.cpp"
    "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory.cpp"
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
        "infra/"
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
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "application/pipeline"
        "application/parser"
        "application/workflow"
        "application/workflow_handler.cpp"
        "application/workflow_handler.hpp"
        "application/workflow_handler_import_flow.module.cpp"
        "application/workflow_handler_entry.module.cpp"
        "application/workflow_handler_error_mapping.module.cpp"
        "application/workflow_handler_stats_logging.module.cpp"
        "application/use_cases/pipeline_api.cpp"
        "application/use_cases/tracer_core_api_pipeline.module.cpp"
    FORBIDDEN_PATTERNS
        ${TT_FORBIDDEN_NON_OWNER_INCLUDE_PATTERNS}
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.exchange\\."
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/exchange/"
)
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "application/query/tree"
        "application/use_cases/query_api.cpp"
        "application/use_cases/tracer_core_api_query.module.cpp"
        "infra/query"
        "infra/query/data/repository/query_runtime_service.cpp"
        "infra/query/data/repository/query_runtime_service_request.cpp"
        "infra/query/data/repository/query_runtime_service_report_mapping.cpp"
        "infra/query/data/repository/query_runtime_service_dispatch.cpp"
        "infra/query/data/repository/query_runtime_service_period.cpp"
        "infra/query/data/repository/query_runtime_service_internal.hpp"
    FORBIDDEN_PATTERNS
        ${TT_FORBIDDEN_NON_OWNER_INCLUDE_PATTERNS}
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.reporting\\."
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/reporting/"
)
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "application/reporting"
        "application/use_cases/report_api.cpp"
        "application/use_cases/report_api_support.cpp"
        "application/use_cases/tracer_core_api_report.module.cpp"
        "infra/reporting"
    FORBIDDEN_PATTERNS
        ${TT_FORBIDDEN_AGGREGATE_INCLUDE_PATTERNS}
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.query\\."
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/query/"
)
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "infra/config"
    FORBIDDEN_PATTERNS
        ${TT_FORBIDDEN_NON_OWNER_INCLUDE_PATTERNS}
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.pipeline\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.exchange\\."
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/pipeline/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/exchange/"
)
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "infra/persistence/importer"
        "infra/persistence/repositories"
        "infra/persistence/sqlite_time_sheet_repository.module.cpp"
        "infra/persistence/sqlite_time_sheet_repository.hpp"
        "infra/persistence/sqlite_database_health_checker.module.cpp"
        "infra/persistence/sqlite_database_health_checker.hpp"
        "infra/persistence/sqlite/db_manager.cpp"
        "infra/persistence/sqlite/db_manager.hpp"
    FORBIDDEN_PATTERNS
        ${TT_FORBIDDEN_NON_OWNER_INCLUDE_PATTERNS}
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.exchange\\."
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/exchange/"
)
enforce_source_content_boundary(
    ROOT "${TRACER_CORE_LIB_SOURCE_ROOT}"
    TARGET_DIRS
        "infra/exchange"
        "infra/crypto"
    FORBIDDEN_PATTERNS
        ${TT_FORBIDDEN_NON_OWNER_INCLUDE_PATTERNS}
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.pipeline\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.application\\.reporting\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.query\\."
        "^[ \t]*import[ \t]+tracer\\.core\\.infrastructure\\.reporting\\."
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/pipeline/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]application/reporting/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/query/"
        "^[ \t]*#[ \t]*include[ \t]*[<\"]infra/reporting/"
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
)

# 3. Infrastructure composition
# - `tracer_transport` / `tracer_adapters_io` stay as standalone adapter libs.
# - `libs/tracer_core/infra` owns capability-aligned infrastructure assembly.
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_transport"
    "${CMAKE_BINARY_DIR}/libs/tracer_transport"
)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_adapters_io"
    "${CMAKE_BINARY_DIR}/libs/tracer_adapters_io"
)
add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_core/infra"
    "${CMAKE_BINARY_DIR}/libs/tracer_core/infra"
)

if(NOT TARGET tc_infra_full_lib)
    message(FATAL_ERROR "Required target missing: tc_infra_full_lib")
endif()
if(NOT TARGET tc_adapters_iface)
    message(FATAL_ERROR "Required target missing: tc_adapters_iface")
endif()
if(NOT TARGET tc_cap_pipeline_lib)
    message(FATAL_ERROR "Required target missing: tc_cap_pipeline_lib")
endif()
if(NOT TARGET tc_cap_query_lib)
    message(FATAL_ERROR "Required target missing: tc_cap_query_lib")
endif()
if(NOT TARGET tc_cap_reporting_lib)
    message(FATAL_ERROR "Required target missing: tc_cap_reporting_lib")
endif()
if(NOT TARGET tc_cap_config_lib)
    message(FATAL_ERROR "Required target missing: tc_cap_config_lib")
endif()
if(NOT TARGET tc_cap_exchange_lib)
    message(FATAL_ERROR "Required target missing: tc_cap_exchange_lib")
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
        tc_infra_reporting_lib
        tc_infra_query_lib
        tc_infra_config_lib
        tc_infra_exchange_lib
        tc_infra_persistence_write_lib
        tc_infra_persistence_runtime_lib
        tc_cap_pipeline_lib
        tc_cap_query_lib
        tc_cap_reporting_lib
        tc_cap_config_lib
        tc_cap_exchange_lib
        tc_rpt_shared_lib
        tc_rpt_data_lib
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
        "Use apps/cli/windows for desktop executable delivery."
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

        add_executable(tc_app_aggregate_runtime_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/application_aggregate_runtime_smoke_tests.cpp"
        )
        setup_app_target(tc_app_aggregate_runtime_smoke_tests NO_PCH)
        target_link_libraries(
            tc_app_aggregate_runtime_smoke_tests PRIVATE
            time_tracker_application
        )
        add_test(
            NAME tc_app_aggregate_runtime_smoke_tests
            COMMAND tc_app_aggregate_runtime_smoke_tests
        )

        add_executable(tc_app_pipeline_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/application_pipeline_module_smoke_tests.cpp"
        )
        setup_app_target(tc_app_pipeline_mod_smoke_tests NO_PCH)
        target_link_libraries(
            tc_app_pipeline_mod_smoke_tests PRIVATE
            time_tracker_application
        )
        add_test(
            NAME tc_app_pipeline_mod_smoke_tests
            COMMAND tc_app_pipeline_mod_smoke_tests
        )

        add_executable(tc_app_query_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/application_query_tree_module_smoke_tests.cpp"
        )
        setup_app_target(tc_app_query_mod_smoke_tests NO_PCH)
        target_link_libraries(
            tc_app_query_mod_smoke_tests PRIVATE
            time_tracker_application
        )
        add_test(
            NAME tc_app_query_mod_smoke_tests
            COMMAND tc_app_query_mod_smoke_tests
        )

        add_executable(tc_app_workflow_mod_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/application_workflow_module_smoke_tests.cpp"
        )
        setup_app_target(tc_app_workflow_mod_smoke_tests NO_PCH)
        target_link_libraries(
            tc_app_workflow_mod_smoke_tests PRIVATE
            time_tracker_application
        )
        add_test(
            NAME tc_app_workflow_mod_smoke_tests
            COMMAND tc_app_workflow_mod_smoke_tests
        )

        add_executable(tc_query_infra_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/infrastructure_modules_smoke_query_main.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/query_stats_repository.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/query_internal_orchestrators.cpp"
        )
        setup_app_target(tc_query_infra_smoke_tests NO_PCH)
        target_include_directories(tc_query_infra_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(tc_query_infra_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_query_infra_smoke_tests
            COMMAND tc_query_infra_smoke_tests
        )

        add_executable(tc_reporting_infra_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/infrastructure_modules_smoke_reporting_main.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/reports.cpp"
        )
        setup_app_target(tc_reporting_infra_smoke_tests NO_PCH)
        target_include_directories(tc_reporting_infra_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(tc_reporting_infra_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_reporting_infra_smoke_tests
            COMMAND tc_reporting_infra_smoke_tests
        )

        add_executable(tc_exchange_infra_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/infrastructure_modules_smoke_exchange_main.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/crypto_exchange.cpp"
        )
        setup_app_target(tc_exchange_infra_smoke_tests NO_PCH)
        target_include_directories(tc_exchange_infra_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(tc_exchange_infra_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_exchange_infra_smoke_tests
            COMMAND tc_exchange_infra_smoke_tests
        )

        add_executable(tc_config_infra_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/infrastructure_modules_smoke_config_main.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/logging_platform_config.cpp"
        )
        setup_app_target(tc_config_infra_smoke_tests NO_PCH)
        target_include_directories(tc_config_infra_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(tc_config_infra_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_config_infra_smoke_tests
            COMMAND tc_config_infra_smoke_tests
        )

        add_executable(tc_persistence_runtime_infra_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/infrastructure_modules_smoke_persistence_runtime_main.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/persistence_runtime.cpp"
        )
        setup_app_target(tc_persistence_runtime_infra_smoke_tests NO_PCH)
        target_include_directories(tc_persistence_runtime_infra_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(tc_persistence_runtime_infra_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_persistence_runtime_infra_smoke_tests
            COMMAND tc_persistence_runtime_infra_smoke_tests
        )

        add_executable(tc_persistence_write_infra_smoke_tests
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/infrastructure_modules_smoke_persistence_write_main.cpp"
            "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/modules_smoke/persistence_write.cpp"
        )
        setup_app_target(tc_persistence_write_infra_smoke_tests NO_PCH)
        target_include_directories(tc_persistence_write_infra_smoke_tests PRIVATE
            "${TRACER_CORE_LIB_TESTS_ROOT}"
        )
        target_link_libraries(tc_persistence_write_infra_smoke_tests PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME tc_persistence_write_infra_smoke_tests
            COMMAND tc_persistence_write_infra_smoke_tests
        )

    add_executable(tt_pipeline_api_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/fakes.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/test_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/convert_ingest_validate_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/import_service_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/record_time_order_mode_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/test_main_pipeline.cpp"
    )
    setup_app_target(tt_pipeline_api_tests)
    target_include_directories(tt_pipeline_api_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_pipeline_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME tt_pipeline_api_tests
        COMMAND tt_pipeline_api_tests
    )

    add_executable(tt_query_api_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/fakes.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/test_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/data_query_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/test_main_query.cpp"
    )
    setup_app_target(tt_query_api_tests)
    target_include_directories(tt_query_api_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_query_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME tt_query_api_tests
        COMMAND tt_query_api_tests
    )

    add_executable(tt_reporting_api_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/fakes.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/test_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/report_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/test_main_reporting.cpp"
    )
    setup_app_target(tt_reporting_api_tests)
    target_include_directories(tt_reporting_api_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_reporting_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME tt_reporting_api_tests
        COMMAND tt_reporting_api_tests
    )

    add_executable(tt_exchange_api_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/fakes.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/test_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/tracer_exchange_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/test_main_exchange.cpp"
    )
    setup_app_target(tt_exchange_api_tests)
    target_include_directories(tt_exchange_api_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_exchange_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME tt_exchange_api_tests
        COMMAND tt_exchange_api_tests
    )

    add_executable(tt_aggregate_runtime_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/fakes.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/support/test_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/modules/tracer_core_runtime_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/application/tests/test_main_aggregate_runtime.cpp"
    )
    setup_app_target(tt_aggregate_runtime_tests)
    target_include_directories(tt_aggregate_runtime_tests PRIVATE
        "${TRACER_CORE_LIB_TESTS_ROOT}"
    )
    target_link_libraries(tt_aggregate_runtime_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME tt_aggregate_runtime_tests
        COMMAND tt_aggregate_runtime_tests
    )

    add_executable(tt_android_runtime_shell_smoke_tests
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_fixture_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_bootstrap_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_config_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_io_chart_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_io_report_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_io_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_shell_smoke_test_main.cpp"
    )
    add_executable(tt_android_runtime_config_tests
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_core_config_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_bundle_policy_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_compat_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_config_test_main.cpp"
    )
    add_executable(tt_android_runtime_reporting_tests
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_smoke_fixture_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_reporting_error_report_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_data_layer_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_structure_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_report_consistency_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_reporting_test_main.cpp"
    )
    add_executable(tt_android_runtime_query_tests
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_test_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_period_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_stats_scenario_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_tree_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/data_query/data_query_refactor_stats_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_query_test_main.cpp"
    )
    add_executable(tt_android_runtime_pipeline_regression_tests
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_history_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_ingest_guard_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_pipeline_validation_regression_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/validation_issue_reporter_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/txt_month_header_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_pipeline_regression_test_main.cpp"
    )
    if(NOT ANDROID)
        foreach(tt_android_runtime_target
            tt_android_runtime_shell_smoke_tests
            tt_android_runtime_config_tests
            tt_android_runtime_reporting_tests
            tt_android_runtime_query_tests
            tt_android_runtime_pipeline_regression_tests
        )
            target_sources(${tt_android_runtime_target} PRIVATE
                "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_config_bridge.cpp"
                "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory.cpp"
                "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory_resolver.cpp"
                "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory_catalog.cpp"
            )
        endforeach()
    endif()
    foreach(tt_android_runtime_target
        tt_android_runtime_shell_smoke_tests
        tt_android_runtime_config_tests
        tt_android_runtime_reporting_tests
        tt_android_runtime_query_tests
        tt_android_runtime_pipeline_regression_tests
    )
        setup_app_target(${tt_android_runtime_target})
        target_include_directories(${tt_android_runtime_target} PRIVATE
            "${TRACER_CORE_SHELL_SOURCE_ROOT}"
            "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}"
        )
        target_link_libraries(${tt_android_runtime_target} PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME ${tt_android_runtime_target}
            COMMAND ${tt_android_runtime_target}
        )
    endforeach()

    add_executable(tt_fmt_parity_tests
        "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/report_formatter/report_formatter_parity_fixture_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/report_formatter/report_formatter_parity_md_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/report_formatter/report_formatter_parity_snapshot_support.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/report_formatter/report_formatter_parity_tex_tests.cpp"
        "${TRACER_CORE_LIB_TESTS_ROOT}/infra/tests/report_formatter/report_formatter_parity_typ_tests.cpp"
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

    add_executable(tt_file_crypto_runtime_bridge_tests
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_config_bridge.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory_resolver.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory_catalog.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_roundtrip_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_failure_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_progress_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_interop_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_test_main.cpp"
    )
    add_executable(tt_exchange_runtime_tests
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_config_bridge.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory_resolver.cpp"
        "${TRACER_CORE_SHELL_HOST_ROOT}/bootstrap/android_runtime_factory_catalog.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/android_runtime/android_runtime_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_test_common.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_package_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_export_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_import_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_tests.cpp"
        "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}/infrastructure/tests/file_crypto/file_crypto_exchange_test_main.cpp"
    )
    foreach(tt_file_crypto_target
        tt_file_crypto_runtime_bridge_tests
        tt_exchange_runtime_tests
    )
        setup_app_target(${tt_file_crypto_target})
        target_include_directories(${tt_file_crypto_target} PRIVATE
            "${TRACER_CORE_SHELL_SOURCE_ROOT}"
            "${TRACER_CORE_SHELL_PLATFORM_TESTS_ROOT}"
        )
        target_link_libraries(${tt_file_crypto_target} PRIVATE
            tc_infra_full_lib
        )
        add_test(
            NAME ${tt_file_crypto_target}
            COMMAND ${tt_file_crypto_target}
        )
    endforeach()
endif()




# src/CMakeLists.txt (Coordinator)

# 1. Domain (Pure Logic)
add_subdirectory(domain)

# 2. Application (Business Logic, Pipeline)
add_subdirectory(application)

# Core boundary for cross-platform reuse:
# time_tracer_core := domain + application
add_library(time_tracer_core INTERFACE)
target_link_libraries(time_tracer_core INTERFACE
    time_tracker_domain
    time_tracker_application
)

# Phase 1 layering target name for cross-platform core.
add_library(tracer_core INTERFACE)
target_link_libraries(tracer_core INTERFACE
    time_tracer_core
)

include(CoreBoundaryRules)
enforce_core_include_boundary(
    ROOT "${PROJECT_SOURCE_DIR}/src"
    CORE_DIRS
        "domain"
        "application"
    FORBIDDEN_PREFIXES
        "api/"
        "infrastructure/"
    FORBIDDEN_PATTERNS
        "modules[\\\\/]tracer_adapters_io[\\\\/]"
        "tracer_adapters_io[\\\\/]src[\\\\/]"
)

# 3. Infrastructure (IO, Persistence, Config)
add_subdirectory(infrastructure)

enforce_core_target_link_boundary(
    CORE_TARGETS
        time_tracker_domain
        time_tracker_application
        time_tracer_core
        tracer_core
    FORBIDDEN_TARGETS
        tracer_adapters
        tti
        ttri
        time_tracker_infrastructure
        time_tracker_reports_infrastructure
        time_tracker_io_adapters
)

# 4. API Layer (Interface)
if(ANDROID)
    message(STATUS
        "Android build detected: JNI bridge target is owned by "
        "apps/tracer_android/runtime."
    )
elseif(WIN32)
    message(STATUS
        "apps/time_tracer no longer builds the Windows CLI. "
        "Use apps/tracer_windows_cli for desktop executable delivery."
    )
endif()

add_subdirectory(api/core_c)

if(BUILD_TESTING)
    set(TIME_TRACKER_CORE_API_TEST_SOURCES
        application/tests/support/fakes.cpp
        application/tests/support/test_support.cpp
        application/tests/modules/convert_ingest_validate_tests.cpp
        application/tests/modules/report_tests.cpp
        application/tests/modules/data_query_tests.cpp
        application/tests/modules/import_service_tests.cpp
        application/tests/test_main.cpp
    )
    add_executable(time_tracker_core_api_tests
        ${TIME_TRACKER_CORE_API_TEST_SOURCES}
    )
    setup_app_target(time_tracker_core_api_tests)
    target_link_libraries(time_tracker_core_api_tests PRIVATE
        time_tracker_application
    )
    add_test(
        NAME time_tracker_core_api_tests
        COMMAND time_tracker_core_api_tests
    )

    add_executable(time_tracker_android_runtime_tests
        infrastructure/tests/android_runtime/android_runtime_test_common.cpp
        infrastructure/tests/android_runtime/android_runtime_smoke_tests.cpp
        infrastructure/tests/android_runtime/android_runtime_core_config_tests.cpp
        infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp
        infrastructure/tests/android_runtime/android_runtime_bundle_policy_tests.cpp
        infrastructure/tests/android_runtime/android_runtime_compat_tests.cpp
        infrastructure/tests/validation_issue_reporter_tests.cpp
        infrastructure/tests/txt_month_header_tests.cpp
        infrastructure/tests/data_query_refactor_tests.cpp
        infrastructure/tests/android_runtime/android_runtime_test_main.cpp
    )
    if(NOT ANDROID)
        target_sources(time_tracker_android_runtime_tests PRIVATE
            api/android/android_runtime_factory.cpp
            api/android/android_runtime_factory_resolver.cpp
            api/android/android_runtime_factory_catalog.cpp
        )
    endif()
    setup_app_target(time_tracker_android_runtime_tests)
    target_link_libraries(time_tracker_android_runtime_tests PRIVATE
        time_tracker_infrastructure
    )
    add_test(
        NAME time_tracker_android_runtime_tests
        COMMAND time_tracker_android_runtime_tests
    )

    add_executable(time_tracker_formatter_parity_tests
        infrastructure/tests/report_formatter_parity_tests.cpp
    )
    setup_app_target(time_tracker_formatter_parity_tests)
    target_link_libraries(time_tracker_formatter_parity_tests PRIVATE
        time_tracker_infrastructure
    )
    add_test(
        NAME time_tracker_formatter_parity_tests
        COMMAND time_tracker_formatter_parity_tests
    )
endif()

add_library(taio_lib STATIC
    src/infrastructure/io/file_io_service.cpp
    src/infrastructure/io/txt_ingest_input_provider.cpp
    src/infrastructure/io/core/file_reader.cpp
    src/infrastructure/io/core/file_writer.cpp
    src/infrastructure/io/core/file_system_helper.cpp
    src/infrastructure/io/utils/file_utils.cpp
)

set(TRACER_CORE_LIB_SOURCE_ROOT "${PROJECT_SOURCE_DIR}/../../libs/tracer_core/src")

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(taio_lib PUBLIC
        FILE_SET adapters_io_modules TYPE CXX_MODULES FILES
            src/modules/tracer.adapters.io.core.reader.cppm
            src/modules/tracer.adapters.io.core.writer.cppm
            src/modules/tracer.adapters.io.core.fs.cppm
            src/modules/tracer.adapters.io.utils.file_utils.cppm
            src/modules/tracer.adapters.io.cppm
    )
    set_target_properties(taio_lib PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

if(TT_ENABLE_PROCESSED_JSON_IO)
    target_sources(taio_lib PRIVATE
        src/infrastructure/io/processed_data_io.cpp
        src/infrastructure/io/processed_json_validation.cpp
        "${TRACER_CORE_LIB_SOURCE_ROOT}/infrastructure/serialization/json_serializer.cpp"
    )
else()
    target_sources(taio_lib PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/io/processed_data_io_noop.cpp"
    )
endif()

if(COMMAND setup_app_target)
    setup_app_target(taio_lib NO_APP_SOURCE_ROOT_INCLUDE)
endif()

target_include_directories(taio_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${TRACER_CORE_LIB_SOURCE_ROOT}"
)

target_link_libraries(taio_lib PUBLIC
    ttr_lib
    tc_all_lib
)

if(TT_ENABLE_PROCESSED_JSON_IO)
    target_link_libraries(taio_lib PUBLIC
        nlohmann_json::nlohmann_json
    )
endif()

if(BUILD_TESTING AND TT_CPP20_MODULES_EFFECTIVE)
    add_executable(taio_mod_smoke_tests
        tests/adapters_io_modules_smoke_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(taio_mod_smoke_tests NO_APP_SOURCE_ROOT_INCLUDE)
    endif()
    target_link_libraries(taio_mod_smoke_tests PRIVATE
        taio_lib
    )
    add_test(
        NAME taio_mod_smoke_tests
        COMMAND taio_mod_smoke_tests
    )

    add_executable(taio_legacy_hdr_compat_tests
        tests/adapters_io_legacy_headers_compat_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(taio_legacy_hdr_compat_tests NO_APP_SOURCE_ROOT_INCLUDE)
    endif()
    target_link_libraries(taio_legacy_hdr_compat_tests PRIVATE
        taio_lib
    )
    add_test(
        NAME taio_legacy_hdr_compat_tests
        COMMAND taio_legacy_hdr_compat_tests
    )
endif()




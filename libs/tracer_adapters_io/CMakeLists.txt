add_library(tracer_adapters_io STATIC
    src/infrastructure/io/file_io_service.cpp
    src/infrastructure/io/txt_ingest_input_provider.cpp
    src/infrastructure/io/core/file_reader.cpp
    src/infrastructure/io/core/file_writer.cpp
    src/infrastructure/io/core/file_system_helper.cpp
    src/infrastructure/io/utils/file_utils.cpp
)

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(tracer_adapters_io PUBLIC
        FILE_SET adapters_io_modules TYPE CXX_MODULES FILES
            src/modules/tracer.adapters.io.core.reader.cppm
            src/modules/tracer.adapters.io.core.writer.cppm
            src/modules/tracer.adapters.io.core.fs.cppm
            src/modules/tracer.adapters.io.utils.file_utils.cppm
            src/modules/tracer.adapters.io.cppm
    )
    set_target_properties(tracer_adapters_io PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

if(TT_ENABLE_PROCESSED_JSON_IO)
    target_sources(tracer_adapters_io PRIVATE
        src/infrastructure/io/processed_data_io.cpp
        "${PROJECT_SOURCE_DIR}/src/infrastructure/serialization/json_serializer.cpp"
        "${PROJECT_SOURCE_DIR}/src/infrastructure/serialization/core/log_codec.cpp"
    )
else()
    target_sources(tracer_adapters_io PRIVATE
        "${PROJECT_SOURCE_DIR}/src/infrastructure/io/processed_data_io_noop.cpp"
    )
endif()

if(COMMAND setup_app_target)
    setup_app_target(tracer_adapters_io)
endif()

target_include_directories(tracer_adapters_io PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(tracer_adapters_io PUBLIC
    tracer_transport
    tracer_core
)

if(TT_ENABLE_PROCESSED_JSON_IO)
    target_link_libraries(tracer_adapters_io PUBLIC
        nlohmann_json::nlohmann_json
    )
endif()

if(BUILD_TESTING AND TT_CPP20_MODULES_EFFECTIVE)
    add_executable(tracer_adapters_io_modules_smoke_tests
        tests/adapters_io_modules_smoke_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(tracer_adapters_io_modules_smoke_tests)
    endif()
    target_link_libraries(tracer_adapters_io_modules_smoke_tests PRIVATE
        tracer_adapters_io
    )
    add_test(
        NAME tracer_adapters_io_modules_smoke_tests
        COMMAND tracer_adapters_io_modules_smoke_tests
    )

    add_executable(tracer_adapters_io_legacy_headers_compat_tests
        tests/adapters_io_legacy_headers_compat_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(tracer_adapters_io_legacy_headers_compat_tests)
    endif()
    target_link_libraries(tracer_adapters_io_legacy_headers_compat_tests PRIVATE
        tracer_adapters_io
    )
    add_test(
        NAME tracer_adapters_io_legacy_headers_compat_tests
        COMMAND tracer_adapters_io_legacy_headers_compat_tests
    )
endif()

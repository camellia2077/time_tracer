add_library(tracer_transport STATIC
    src/envelope.cpp
    src/errors.cpp
    src/fields.cpp
    src/runtime_codec_ingest.cpp
    src/runtime_codec_runtime.cpp
    src/runtime_codec_capabilities.cpp
    src/runtime_codec_workflow.cpp
    src/runtime_codec_query.cpp
    src/runtime_codec_report.cpp
    src/runtime_codec_export.cpp
    src/runtime_codec_tree.cpp
)

if(COMMAND setup_app_target)
    setup_app_target(tracer_transport)
endif()

target_include_directories(tracer_transport PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(tracer_transport
    PRIVATE nlohmann_json::nlohmann_json
)

if(BUILD_TESTING)
    add_executable(tracer_transport_tests
        tests/transport_envelope_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(tracer_transport_tests)
    endif()
    target_link_libraries(tracer_transport_tests PRIVATE
        tracer_transport
    )
    add_test(
        NAME tracer_transport_tests
        COMMAND tracer_transport_tests
    )

    add_executable(tracer_transport_fields_tests
        tests/transport_fields_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(tracer_transport_fields_tests)
    endif()
    target_link_libraries(tracer_transport_fields_tests PRIVATE
        tracer_transport
    )
    add_test(
        NAME tracer_transport_fields_tests
        COMMAND tracer_transport_fields_tests
    )

    add_executable(tracer_transport_runtime_codec_tests
        tests/transport_runtime_codec_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(tracer_transport_runtime_codec_tests)
    endif()
    target_link_libraries(tracer_transport_runtime_codec_tests PRIVATE
        tracer_transport
    )
    add_test(
        NAME tracer_transport_runtime_codec_tests
        COMMAND tracer_transport_runtime_codec_tests
    )
endif()

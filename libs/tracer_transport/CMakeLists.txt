add_library(ttr_lib STATIC
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

if(TT_CPP20_MODULES_EFFECTIVE)
    target_sources(ttr_lib PUBLIC
        FILE_SET transport_modules TYPE CXX_MODULES FILES
            src/modules/tracer.transport.errors.cppm
            src/modules/tracer.transport.envelope.cppm
            src/modules/tracer.transport.cppm
    )
    set_target_properties(ttr_lib PROPERTIES
        CXX_SCAN_FOR_MODULES ON
    )
endif()

if(COMMAND setup_app_target)
    setup_app_target(ttr_lib)
endif()

target_include_directories(ttr_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(ttr_lib
    PUBLIC nlohmann_json::nlohmann_json
)

if(BUILD_TESTING)
    add_executable(ttr_tests
        tests/transport_envelope_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(ttr_tests)
    endif()
    target_link_libraries(ttr_tests PRIVATE
        ttr_lib
    )
    add_test(
        NAME ttr_tests
        COMMAND ttr_tests
    )

    add_executable(ttr_fields_tests
        tests/transport_fields_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(ttr_fields_tests)
    endif()
    target_link_libraries(ttr_fields_tests PRIVATE
        ttr_lib
    )
    add_test(
        NAME ttr_fields_tests
        COMMAND ttr_fields_tests
    )

    add_executable(ttr_rt_codec_tests
        tests/transport_runtime_codec_tests.cpp
    )
    if(COMMAND setup_app_target)
        setup_app_target(ttr_rt_codec_tests)
    endif()
    target_link_libraries(ttr_rt_codec_tests PRIVATE
        ttr_lib
    )
    add_test(
        NAME ttr_rt_codec_tests
        COMMAND ttr_rt_codec_tests
    )

    if(TT_CPP20_MODULES_EFFECTIVE)
        add_executable(ttr_mod_smoke_tests
            tests/transport_modules_smoke_tests.cpp
        )
        if(COMMAND setup_app_target)
            setup_app_target(ttr_mod_smoke_tests)
        endif()
        target_link_libraries(ttr_mod_smoke_tests PRIVATE
            ttr_lib
        )
        add_test(
            NAME ttr_mod_smoke_tests
            COMMAND ttr_mod_smoke_tests
        )
    endif()
endif()

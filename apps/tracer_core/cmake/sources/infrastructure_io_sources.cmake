set(TIME_TRACKER_INFRA_IO_SOURCES
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/file_io_service.cpp"
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/txt_ingest_input_provider.cpp"
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/core/file_reader.cpp"
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/core/file_writer.cpp"
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/core/file_system_helper.cpp"
    "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/utils/file_utils.cpp"
)

if(TT_ENABLE_PROCESSED_JSON_IO)
    list(APPEND TIME_TRACKER_INFRA_IO_SOURCES
        "${TRACER_ADAPTERS_IO_SOURCE_ROOT}/infrastructure/io/processed_data_io.cpp"
        "serialization/json_serializer.cpp"
        "serialization/core/log_codec.cpp"
    )
else()
    list(APPEND TIME_TRACKER_INFRA_IO_SOURCES
        "${PROJECT_SOURCE_DIR}/src/infrastructure/io/processed_data_io_noop.cpp"
    )
endif()

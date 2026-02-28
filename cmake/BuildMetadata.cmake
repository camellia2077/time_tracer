include_guard(GLOBAL)

set(TT_BUILD_METADATA_SCRIPT_PATH
    "${CMAKE_CURRENT_LIST_DIR}/GenerateBuildMetadata.cmake"
)

function(tt_attach_build_metadata target)
    set(options)
    set(one_value_args PREFIX HEADER_NAME)
    cmake_parse_arguments(TT_METADATA "${options}" "${one_value_args}" "" ${ARGN})

    if(NOT TARGET "${target}")
        message(FATAL_ERROR
            "tt_attach_build_metadata: target '${target}' does not exist."
        )
    endif()

    if(NOT TT_METADATA_PREFIX)
        message(FATAL_ERROR
            "tt_attach_build_metadata: PREFIX is required."
        )
    endif()

    if(NOT EXISTS "${TT_BUILD_METADATA_SCRIPT_PATH}")
        message(FATAL_ERROR
            "tt_attach_build_metadata: generator script not found: "
            "${TT_BUILD_METADATA_SCRIPT_PATH}"
        )
    endif()

    if(TT_METADATA_HEADER_NAME)
        set(_metadata_header_name "${TT_METADATA_HEADER_NAME}")
    else()
        string(TOLOWER "${TT_METADATA_PREFIX}" _metadata_prefix_lower)
        set(_metadata_header_name "${_metadata_prefix_lower}_build_metadata.hpp")
    endif()

    set(_metadata_output_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/build_metadata")
    set(_metadata_header_path "${_metadata_output_dir}/${_metadata_header_name}")
    string(MAKE_C_IDENTIFIER
        "${TT_METADATA_PREFIX}_BUILD_METADATA_GENERATED_HPP_"
        _metadata_header_guard
    )

    set(_metadata_target "${target}_${TT_METADATA_PREFIX}_build_metadata")
    add_custom_target("${_metadata_target}" ALL
        COMMAND "${CMAKE_COMMAND}"
            "-DOUTPUT_PATH=${_metadata_header_path}"
            "-DHEADER_GUARD=${_metadata_header_guard}"
            "-DDATE_MACRO=${TT_METADATA_PREFIX}_BUILD_DATE"
            "-DTIMESTAMP_MACRO=${TT_METADATA_PREFIX}_BUILD_TIMESTAMP"
            -P "${TT_BUILD_METADATA_SCRIPT_PATH}"
        BYPRODUCTS "${_metadata_header_path}"
        COMMENT "Generating build metadata header for ${target}"
        VERBATIM
    )

    add_dependencies("${target}" "${_metadata_target}")
    target_include_directories("${target}" PUBLIC
        "${_metadata_output_dir}"
    )
endfunction()

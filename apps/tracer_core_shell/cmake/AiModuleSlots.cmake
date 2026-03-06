# Optional AI module insertion points.
#
# Dependency direction:
# - tracer_core_ai -> tracer_core
# - tracer_ai_provider -> tracer_core_ai
# - tracer_core does not depend on AI/provider modules

function(_tt_ensure_interface_slot TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        add_library(${TARGET_NAME} INTERFACE)
    endif()
endfunction()

function(tt_configure_optional_ai_modules)
    _tt_ensure_interface_slot(tt_core_ai_slot)
    _tt_ensure_interface_slot(tt_ai_provider_slot)

    if(TT_ENABLE_AI_JSON_EFFECTIVE)
        add_subdirectory(
            "${TT_TRACER_CORE_AI_SOURCE_DIR}"
            "${CMAKE_BINARY_DIR}/libs/tracer_core_ai"
        )
        if(NOT TARGET tracer_core_ai)
            message(FATAL_ERROR
                "Optional module at ${TT_TRACER_CORE_AI_SOURCE_DIR} must "
                "define target `tracer_core_ai`."
            )
        endif()
        target_link_libraries(tt_core_ai_slot INTERFACE tracer_core_ai)
    endif()

    if(TT_ENABLE_AI_PROVIDER_EFFECTIVE)
        add_subdirectory(
            "${TT_TRACER_AI_PROVIDER_SOURCE_DIR}"
            "${CMAKE_BINARY_DIR}/libs/tracer_ai_provider"
        )
        if(NOT TARGET tracer_ai_provider)
            message(FATAL_ERROR
                "Optional module at ${TT_TRACER_AI_PROVIDER_SOURCE_DIR} must "
                "define target `tracer_ai_provider`."
            )
        endif()
        target_link_libraries(tt_ai_provider_slot INTERFACE tracer_ai_provider)
    endif()
endfunction()

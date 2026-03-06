# AI extension options and effective module availability.

if(WIN32 OR ANDROID)
    set(TT_ENABLE_AI_JSON_DEFAULT ON)
    set(TT_ENABLE_AI_PROVIDER_DEFAULT ON)
else()
    set(TT_ENABLE_AI_JSON_DEFAULT OFF)
    set(TT_ENABLE_AI_PROVIDER_DEFAULT OFF)
endif()

option(TT_ENABLE_AI_JSON
       "Enable optional tracer_core_ai semantic AI extension when present"
       ${TT_ENABLE_AI_JSON_DEFAULT})
option(TT_ENABLE_AI_PROVIDER
       "Enable optional tracer_ai_provider adapter when present"
       ${TT_ENABLE_AI_PROVIDER_DEFAULT})

unset(TT_ENABLE_AI_JSON_DEFAULT)
unset(TT_ENABLE_AI_PROVIDER_DEFAULT)

set(TT_TRACER_CORE_AI_SOURCE_DIR
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_core_ai"
    CACHE PATH "Path to optional tracer_core_ai module")
set(TT_TRACER_AI_PROVIDER_SOURCE_DIR
    "${PROJECT_SOURCE_DIR}/../../libs/tracer_ai_provider"
    CACHE PATH "Path to optional tracer_ai_provider module")

if(EXISTS "${TT_TRACER_CORE_AI_SOURCE_DIR}/CMakeLists.txt")
    set(TT_AI_JSON_MODULE_AVAILABLE ON)
else()
    set(TT_AI_JSON_MODULE_AVAILABLE OFF)
endif()

if(EXISTS "${TT_TRACER_AI_PROVIDER_SOURCE_DIR}/CMakeLists.txt")
    set(TT_AI_PROVIDER_MODULE_AVAILABLE ON)
else()
    set(TT_AI_PROVIDER_MODULE_AVAILABLE OFF)
endif()

set(TT_ENABLE_AI_JSON_EFFECTIVE OFF)
if(TT_ENABLE_AI_JSON)
    if(TT_AI_JSON_MODULE_AVAILABLE)
        set(TT_ENABLE_AI_JSON_EFFECTIVE ON)
    else()
        message(STATUS
            "TT_ENABLE_AI_JSON=ON but optional module is absent: "
            "${TT_TRACER_CORE_AI_SOURCE_DIR}"
        )
    endif()
endif()

set(TT_ENABLE_AI_PROVIDER_EFFECTIVE OFF)
if(TT_ENABLE_AI_PROVIDER)
    if(NOT TT_ENABLE_AI_JSON_EFFECTIVE)
        message(STATUS
            "TT_ENABLE_AI_PROVIDER requested but tracer_core_ai is not "
            "effective; provider slot remains reserved only."
        )
    elseif(TT_AI_PROVIDER_MODULE_AVAILABLE)
        set(TT_ENABLE_AI_PROVIDER_EFFECTIVE ON)
    else()
        message(STATUS
            "TT_ENABLE_AI_PROVIDER=ON but optional module is absent: "
            "${TT_TRACER_AI_PROVIDER_SOURCE_DIR}"
        )
    endif()
endif()

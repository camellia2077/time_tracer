# Shared feature/build interface targets.

function(_tt_define_bool_feature TARGET_NAME MACRO_NAME ENABLED_VALUE)
    if(ENABLED_VALUE)
        target_compile_definitions(${TARGET_NAME} INTERFACE "${MACRO_NAME}=1")
    else()
        target_compile_definitions(${TARGET_NAME} INTERFACE "${MACRO_NAME}=0")
    endif()
endfunction()

if(NOT TARGET tt_build_options)
    add_library(tt_build_options INTERFACE)
    target_compile_features(tt_build_options INTERFACE cxx_std_23)
endif()

if(NOT TARGET tt_feature_flags)
    add_library(tt_feature_flags INTERFACE)
endif()

_tt_define_bool_feature(
    tt_feature_flags
    TT_ENABLE_PROCESSED_JSON_IO
    "${TT_ENABLE_PROCESSED_JSON_IO}"
)
_tt_define_bool_feature(
    tt_feature_flags
    TT_REPORT_ENABLE_LATEX
    "${TT_REPORT_ENABLE_LATEX}"
)
_tt_define_bool_feature(
    tt_feature_flags
    TT_REPORT_ENABLE_TYPST
    "${TT_REPORT_ENABLE_TYPST}"
)
_tt_define_bool_feature(
    tt_feature_flags
    TT_ENABLE_HEAVY_DIAGNOSTICS
    "${TT_ENABLE_HEAVY_DIAGNOSTICS}"
)
_tt_define_bool_feature(
    tt_feature_flags
    TT_ENABLE_AI_JSON
    "${TT_ENABLE_AI_JSON_EFFECTIVE}"
)
_tt_define_bool_feature(
    tt_feature_flags
    TT_ENABLE_AI_PROVIDER
    "${TT_ENABLE_AI_PROVIDER_EFFECTIVE}"
)

target_compile_definitions(tt_feature_flags INTERFACE
    TT_ENABLE_FILE_CRYPTO=1
)

if(TT_USE_BUNDLED_SQLITE)
    set(TT_RUNTIME_REQUIRE_SQLITE_DLL OFF)
else()
    set(TT_RUNTIME_REQUIRE_SQLITE_DLL ON)
endif()

if(TT_TOML_HEADER_ONLY)
    set(TT_RUNTIME_REQUIRE_TOML_DLL OFF)
else()
    set(TT_RUNTIME_REQUIRE_TOML_DLL ON)
endif()

_tt_define_bool_feature(
    tt_feature_flags
    TT_RUNTIME_REQUIRE_SQLITE_DLL
    "${TT_RUNTIME_REQUIRE_SQLITE_DLL}"
)
_tt_define_bool_feature(
    tt_feature_flags
    TT_RUNTIME_REQUIRE_TOML_DLL
    "${TT_RUNTIME_REQUIRE_TOML_DLL}"
)

if(WIN32 AND MINGW AND
   (NOT TT_STATIC_MINGW_RUNTIME OR NOT TT_STATIC_MINGW_RUNTIME_PLUGINS))
    target_compile_definitions(tt_feature_flags INTERFACE
        TT_RUNTIME_REQUIRE_MINGW_DLLS=1
    )
else()
    target_compile_definitions(tt_feature_flags INTERFACE
        TT_RUNTIME_REQUIRE_MINGW_DLLS=0
    )
endif()

if(TARGET sodium)
    target_compile_definitions(tt_feature_flags INTERFACE TT_HAS_LIBSODIUM=1)
else()
    message(FATAL_ERROR "Required target missing: sodium")
endif()

if(TARGET zstd::libzstd)
    target_compile_definitions(tt_feature_flags INTERFACE TT_HAS_ZSTD=1)
else()
    message(FATAL_ERROR "Required target missing: zstd::libzstd")
endif()

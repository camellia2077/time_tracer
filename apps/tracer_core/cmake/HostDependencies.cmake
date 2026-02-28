include_guard(GLOBAL)

include(FetchContent)

function(_tt_host_define_sqlite_aliases_if_needed)
    if(TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
        add_library(SQLite3 INTERFACE IMPORTED)
        target_link_libraries(SQLite3 INTERFACE SQLite::SQLite3)
        return()
    endif()

    if(TARGET SQLite3 AND NOT TARGET SQLite::SQLite3)
        add_library(SQLite::SQLite3 INTERFACE IMPORTED)
        target_link_libraries(SQLite::SQLite3 INTERFACE SQLite3)
    endif()
endfunction()

function(_tt_host_ensure_nlohmann_json)
    if(TARGET nlohmann_json::nlohmann_json)
        return()
    endif()

    find_package(nlohmann_json REQUIRED)
    if(NOT TARGET nlohmann_json::nlohmann_json)
        message(FATAL_ERROR "Host dependency missing: nlohmann_json::nlohmann_json")
    endif()
endfunction()

function(_tt_host_ensure_sqlite3)
    if(TARGET SQLite::SQLite3 OR TARGET SQLite3)
        _tt_host_define_sqlite_aliases_if_needed()
        return()
    endif()

    if(WIN32 AND TT_USE_BUNDLED_SQLITE)
        message(STATUS "Windows host: using bundled sqlite amalgamation.")
        FetchContent_Declare(
            time_tracker_sqlite_amalgamation
            URL "https://www.sqlite.org/2026/sqlite-amalgamation-3510200.zip"
            URL_HASH "SHA256=6E2A845A493026BDBAD0618B2B5A0CF48584FAAB47384480ED9F592D912F23EC"
        )
        FetchContent_MakeAvailable(time_tracker_sqlite_amalgamation)

        add_library(time_tracker_sqlite3 STATIC
            "${time_tracker_sqlite_amalgamation_SOURCE_DIR}/sqlite3.c"
        )
        target_include_directories(time_tracker_sqlite3 PUBLIC
            "${time_tracker_sqlite_amalgamation_SOURCE_DIR}"
        )
        target_compile_definitions(time_tracker_sqlite3 PUBLIC
            SQLITE_THREADSAFE=1
            SQLITE_OMIT_LOAD_EXTENSION
        )

        add_library(SQLite::SQLite3 ALIAS time_tracker_sqlite3)
        add_library(SQLite3 ALIAS time_tracker_sqlite3)
        return()
    endif()

    find_package(SQLite3 REQUIRED)
    _tt_host_define_sqlite_aliases_if_needed()

    if(NOT TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
        message(FATAL_ERROR "Host dependency missing: SQLite3")
    endif()
endfunction()

function(_tt_host_ensure_tomlplusplus)
    if(TARGET tomlplusplus::tomlplusplus)
        return()
    endif()

    if(WIN32 AND TT_TOML_HEADER_ONLY)
        message(STATUS "Windows host: using bundled tomlplusplus header-only.")
        FetchContent_Declare(
            time_tracker_tomlplusplus
            URL "https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.tar.gz"
            URL_HASH "SHA256=8517F65938A4FAAE9CCF8EBB36631A38C1CADFB5EFA85D9A72E15B9E97D25155"
        )
        FetchContent_MakeAvailable(time_tracker_tomlplusplus)

        if(TARGET tomlplusplus::tomlplusplus)
            return()
        endif()

        add_library(time_tracker_tomlplusplus INTERFACE)
        target_include_directories(time_tracker_tomlplusplus INTERFACE
            "${time_tracker_tomlplusplus_SOURCE_DIR}/include"
        )
        target_compile_definitions(time_tracker_tomlplusplus INTERFACE
            TOML_HEADER_ONLY=1
        )

        add_library(tomlplusplus::tomlplusplus ALIAS time_tracker_tomlplusplus)
        return()
    endif()

    find_package(tomlplusplus REQUIRED)
    if(NOT TARGET tomlplusplus::tomlplusplus)
        message(FATAL_ERROR "Host dependency missing: tomlplusplus::tomlplusplus")
    endif()
endfunction()

function(_tt_host_ensure_libsodium)
    if(TARGET sodium)
        return()
    endif()

    find_package(sodium QUIET)
    if(TARGET sodium)
        return()
    endif()

    find_package(unofficial-sodium CONFIG QUIET)
    if(TARGET unofficial-sodium::sodium)
        add_library(sodium ALIAS unofficial-sodium::sodium)
        return()
    endif()

    message(STATUS "Host: fetching libsodium via libsodium-cmake.")
    set(SODIUM_DISABLE_TESTS ON CACHE BOOL "" FORCE)
    set(SODIUM_MINIMAL ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        libsodium_cmake
        GIT_REPOSITORY "https://github.com/robinlinden/libsodium-cmake.git"
        GIT_TAG "260622e5b69bce9b955603a98e46354125a932a4"
    )
    FetchContent_MakeAvailable(libsodium_cmake)

    if(NOT TARGET sodium)
        message(FATAL_ERROR "Host dependency missing: sodium (libsodium)")
    endif()
endfunction()

function(_tt_host_define_zstd_alias_if_needed)
    if(TARGET zstd::libzstd)
        return()
    endif()

    if(TARGET libzstd_static)
        add_library(zstd::libzstd ALIAS libzstd_static)
        return()
    endif()

    if(TARGET libzstd_shared)
        add_library(zstd::libzstd ALIAS libzstd_shared)
        return()
    endif()

    if(TARGET zstd::zstd)
        add_library(zstd::libzstd ALIAS zstd::zstd)
        return()
    endif()

    if(TARGET zstd_static)
        add_library(zstd::libzstd ALIAS zstd_static)
    endif()
endfunction()

function(_tt_host_ensure_zstd)
    if(TARGET zstd::libzstd OR TARGET libzstd_static OR TARGET libzstd_shared
       OR TARGET zstd::zstd OR TARGET zstd_static)
        _tt_host_define_zstd_alias_if_needed()
        return()
    endif()

    find_package(zstd CONFIG QUIET)
    if(TARGET zstd::libzstd OR TARGET libzstd_static OR TARGET libzstd_shared
       OR TARGET zstd::zstd OR TARGET zstd_static)
        _tt_host_define_zstd_alias_if_needed()
        return()
    endif()

    message(STATUS "Host: fetching zstd from upstream source.")
    set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(ZSTD_LEGACY_SUPPORT OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        zstd_upstream
        GIT_REPOSITORY "https://github.com/facebook/zstd.git"
        GIT_TAG "v1.5.7"
        SOURCE_SUBDIR "build/cmake"
    )
    FetchContent_MakeAvailable(zstd_upstream)
    _tt_host_define_zstd_alias_if_needed()

    if(NOT TARGET zstd::libzstd)
        message(FATAL_ERROR "Host dependency missing: zstd::libzstd")
    endif()
endfunction()

function(configure_time_tracer_host_dependencies)
    _tt_host_ensure_nlohmann_json()
    _tt_host_ensure_sqlite3()
    _tt_host_ensure_tomlplusplus()
    _tt_host_ensure_libsodium()
    _tt_host_ensure_zstd()
endfunction()

include_guard(GLOBAL)

include(FetchContent)

function(_tt_ensure_nlohmann_json)
    if(TARGET nlohmann_json::nlohmann_json)
        return()
    endif()

    find_package(nlohmann_json QUIET)
    if(TARGET nlohmann_json::nlohmann_json)
        return()
    endif()

    message(STATUS "Android: fetching nlohmann_json.")
    FetchContent_Declare(
        nlohmann_json
        URL "https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz"
        URL_HASH "SHA256=42F6E95CAD6EC532FD372391373363B62A14AF6D771056DBFC86160E6DFFF7AA"
    )
    FetchContent_MakeAvailable(nlohmann_json)
endfunction()

function(_tt_ensure_tomlplusplus)
    if(TARGET tomlplusplus::tomlplusplus)
        return()
    endif()

    find_package(tomlplusplus QUIET)
    if(TARGET tomlplusplus::tomlplusplus)
        return()
    endif()

    message(STATUS "Android: fetching tomlplusplus.")
    FetchContent_Declare(
        tomlplusplus
        URL "https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.tar.gz"
        URL_HASH "SHA256=8517F65938A4FAAE9CCF8EBB36631A38C1CADFB5EFA85D9A72E15B9E97D25155"
    )
    FetchContent_MakeAvailable(tomlplusplus)
endfunction()

function(_tt_ensure_sqlite3)
    if(TARGET SQLite::SQLite3 OR TARGET SQLite3)
        if(TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
            add_library(SQLite3 INTERFACE IMPORTED)
            target_link_libraries(SQLite3 INTERFACE SQLite::SQLite3)
        endif()
        return()
    endif()

    find_package(SQLite3 QUIET)
    if(TARGET SQLite::SQLite3 OR TARGET SQLite3)
        if(TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
            add_library(SQLite3 INTERFACE IMPORTED)
            target_link_libraries(SQLite3 INTERFACE SQLite::SQLite3)
        endif()
        return()
    endif()

    message(STATUS "Android: fetching sqlite amalgamation.")
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
endfunction()

function(configure_time_tracer_android_dependencies)
    _tt_ensure_nlohmann_json()
    _tt_ensure_tomlplusplus()
    _tt_ensure_sqlite3()

    if(NOT TARGET nlohmann_json::nlohmann_json)
        message(FATAL_ERROR "Android dependency missing: nlohmann_json::nlohmann_json")
    endif()
    if(NOT TARGET tomlplusplus::tomlplusplus)
        message(FATAL_ERROR "Android dependency missing: tomlplusplus::tomlplusplus")
    endif()
    if(NOT TARGET SQLite::SQLite3 AND NOT TARGET SQLite3)
        message(FATAL_ERROR "Android dependency missing: SQLite3")
    endif()
endfunction()

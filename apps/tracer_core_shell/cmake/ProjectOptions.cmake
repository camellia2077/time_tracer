# Project-level options, defaults, and cache overrides.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(TT_WINDOWS_DEP_DEFAULT ON)
else()
    set(TT_WINDOWS_DEP_DEFAULT OFF)
endif()

option(TT_USE_BUNDLED_SQLITE
       "Use bundled sqlite amalgamation on Windows host builds"
       ${TT_WINDOWS_DEP_DEFAULT})
option(TT_TOML_HEADER_ONLY
       "Use tomlplusplus as header-only on Windows host builds"
       ${TT_WINDOWS_DEP_DEFAULT})
option(TT_STATIC_MINGW_RUNTIME
       "Prefer static libgcc/libstdc++/winpthread on Windows MinGW builds"
       ${TT_WINDOWS_DEP_DEFAULT})
option(TT_STATIC_MINGW_RUNTIME_PLUGINS
       "Prefer static libgcc/libstdc++/winpthread for plugin/runtime shared libraries on Windows MinGW builds"
       ${TT_STATIC_MINGW_RUNTIME})
option(TT_ENABLE_EXPERIMENTAL_LTO
       "Enable experimental LTO/FTO path for Release builds"
       OFF)
unset(TT_WINDOWS_DEP_DEFAULT)

option(ENABLE_APP_ICON "Enable application icon for Windows executables" OFF)
option(TT_ENABLE_PROCESSED_JSON_IO
       "Enable processed JSON file IO (struct<->json persistence)" ON)
option(TT_REPORT_ENABLE_LATEX
       "Enable LaTeX report formatter implementation in core runtime" ON)
option(TT_REPORT_ENABLE_TYPST
       "Enable Typst report formatter implementation in core runtime" ON)
option(TT_ENABLE_HEAVY_DIAGNOSTICS
       "Enable heavy diagnostics details (diff context/trace payloads)" OFF)

include("${CMAKE_CURRENT_LIST_DIR}/options/ai_options.cmake")

if(WIN32 OR ANDROID)
    set(TT_CPP20_MODULES_DEFAULT ON)
else()
    set(TT_CPP20_MODULES_DEFAULT OFF)
endif()

option(TT_ENABLE_CPP20_MODULES
       "Enable C++20 named modules for selected module targets"
       ${TT_CPP20_MODULES_DEFAULT})
unset(TT_CPP20_MODULES_DEFAULT)

set(TT_CPP20_MODULES_EFFECTIVE OFF)
if(TT_ENABLE_CPP20_MODULES)
    if(CMAKE_VERSION VERSION_LESS 3.28)
        message(WARNING
            "TT_ENABLE_CPP20_MODULES=ON ignored: CMake < 3.28 does not meet "
            "the project modules baseline."
        )
    elseif(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|MSVC")
        message(WARNING
            "TT_ENABLE_CPP20_MODULES=ON ignored: unsupported compiler "
            "(${CMAKE_CXX_COMPILER_ID})."
        )
    else()
        set(TT_CPP20_MODULES_EFFECTIVE ON)
    endif()
endif()

if(ANDROID)
    set(TT_ENABLE_PROCESSED_JSON_IO OFF CACHE BOOL "" FORCE)
    set(TT_REPORT_ENABLE_LATEX OFF CACHE BOOL "" FORCE)
    set(TT_REPORT_ENABLE_TYPST OFF CACHE BOOL "" FORCE)
endif()

set(PLUGIN_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")

find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found, enabling compiler launcher.")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
endif()

message(STATUS "TT_ENABLE_PROCESSED_JSON_IO=${TT_ENABLE_PROCESSED_JSON_IO}")
message(STATUS "TT_REPORT_ENABLE_LATEX=${TT_REPORT_ENABLE_LATEX}")
message(STATUS "TT_REPORT_ENABLE_TYPST=${TT_REPORT_ENABLE_TYPST}")
message(STATUS "TT_ENABLE_HEAVY_DIAGNOSTICS=${TT_ENABLE_HEAVY_DIAGNOSTICS}")
message(STATUS "TT_ENABLE_AI_JSON=${TT_ENABLE_AI_JSON}")
message(STATUS "TT_ENABLE_AI_JSON_EFFECTIVE=${TT_ENABLE_AI_JSON_EFFECTIVE}")
message(STATUS "TT_ENABLE_AI_PROVIDER=${TT_ENABLE_AI_PROVIDER}")
message(STATUS "TT_ENABLE_AI_PROVIDER_EFFECTIVE=${TT_ENABLE_AI_PROVIDER_EFFECTIVE}")
message(STATUS "TT_ENABLE_CPP20_MODULES=${TT_ENABLE_CPP20_MODULES}")
message(STATUS "TT_CPP20_MODULES_EFFECTIVE=${TT_CPP20_MODULES_EFFECTIVE}")
message(STATUS "TT_USE_BUNDLED_SQLITE=${TT_USE_BUNDLED_SQLITE}")
message(STATUS "TT_TOML_HEADER_ONLY=${TT_TOML_HEADER_ONLY}")
message(STATUS "TT_STATIC_MINGW_RUNTIME=${TT_STATIC_MINGW_RUNTIME}")
message(STATUS "TT_STATIC_MINGW_RUNTIME_PLUGINS=${TT_STATIC_MINGW_RUNTIME_PLUGINS}")
message(STATUS "TT_ENABLE_EXPERIMENTAL_LTO=${TT_ENABLE_EXPERIMENTAL_LTO}")

# --- START OF FILE CMakeLists.txt ---
cmake_minimum_required(VERSION 3.25)

project(TimeTrackerApp VERSION "0.5.5" LANGUAGES C CXX)

# [新增] 强制 CMake 的命令行输出使用 UTF-8 编码
set(CMAKE_CLI_FORCE_UTF8 ON)

cmake_policy(SET CMP0115 NEW)

include(CTest)

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
option(TT_ENABLE_EXPERIMENTAL_LTO
       "Enable experimental LTO/FTO path for Release builds"
       OFF)
unset(TT_WINDOWS_DEP_DEFAULT)

# --- Find External Dependencies ---
if(ANDROID)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AndroidDependencies.cmake")
    configure_time_tracer_android_dependencies()
else()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/HostDependencies.cmake")
    configure_time_tracer_host_dependencies()
endif()

# --- 1. 项目范围的设置 ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_APP_ICON "Enable application icon for Windows executables" OFF)
option(TT_ENABLE_PROCESSED_JSON_IO
       "Enable processed JSON file IO (struct<->json persistence)" ON)
option(TT_REPORT_ENABLE_LATEX
       "Enable LaTeX report formatter implementation in core runtime" ON)
option(TT_REPORT_ENABLE_TYPST
       "Enable Typst report formatter implementation in core runtime" ON)
option(TT_ENABLE_HEAVY_DIAGNOSTICS
       "Enable heavy diagnostics details (diff context/trace payloads)" OFF)

if(ANDROID)
    set(TT_ENABLE_PROCESSED_JSON_IO OFF CACHE BOOL "" FORCE)
    set(TT_REPORT_ENABLE_LATEX OFF CACHE BOOL "" FORCE)
    set(TT_REPORT_ENABLE_TYPST OFF CACHE BOOL "" FORCE)
endif()

message(STATUS "TT_ENABLE_PROCESSED_JSON_IO=${TT_ENABLE_PROCESSED_JSON_IO}")
message(STATUS "TT_REPORT_ENABLE_LATEX=${TT_REPORT_ENABLE_LATEX}")
message(STATUS "TT_REPORT_ENABLE_TYPST=${TT_REPORT_ENABLE_TYPST}")
message(STATUS "TT_ENABLE_HEAVY_DIAGNOSTICS=${TT_ENABLE_HEAVY_DIAGNOSTICS}")
message(STATUS "TT_USE_BUNDLED_SQLITE=${TT_USE_BUNDLED_SQLITE}")
message(STATUS "TT_TOML_HEADER_ONLY=${TT_TOML_HEADER_ONLY}")
message(STATUS "TT_STATIC_MINGW_RUNTIME=${TT_STATIC_MINGW_RUNTIME}")
message(STATUS "TT_ENABLE_EXPERIMENTAL_LTO=${TT_ENABLE_EXPERIMENTAL_LTO}")

if(TT_ENABLE_PROCESSED_JSON_IO)
    add_compile_definitions(TT_ENABLE_PROCESSED_JSON_IO=1)
else()
    add_compile_definitions(TT_ENABLE_PROCESSED_JSON_IO=0)
endif()

if(TT_REPORT_ENABLE_LATEX)
    add_compile_definitions(TT_REPORT_ENABLE_LATEX=1)
else()
    add_compile_definitions(TT_REPORT_ENABLE_LATEX=0)
endif()

if(TT_REPORT_ENABLE_TYPST)
    add_compile_definitions(TT_REPORT_ENABLE_TYPST=1)
else()
    add_compile_definitions(TT_REPORT_ENABLE_TYPST=0)
endif()

if(TT_ENABLE_HEAVY_DIAGNOSTICS)
    add_compile_definitions(TT_ENABLE_HEAVY_DIAGNOSTICS=1)
else()
    add_compile_definitions(TT_ENABLE_HEAVY_DIAGNOSTICS=0)
endif()

add_compile_definitions(TT_ENABLE_FILE_CRYPTO=1)

if(TT_USE_BUNDLED_SQLITE)
    add_compile_definitions(TT_RUNTIME_REQUIRE_SQLITE_DLL=0)
else()
    add_compile_definitions(TT_RUNTIME_REQUIRE_SQLITE_DLL=1)
endif()

if(TT_TOML_HEADER_ONLY)
    add_compile_definitions(TT_RUNTIME_REQUIRE_TOML_DLL=0)
else()
    add_compile_definitions(TT_RUNTIME_REQUIRE_TOML_DLL=1)
endif()

if(WIN32 AND MINGW AND NOT TT_STATIC_MINGW_RUNTIME)
    add_compile_definitions(TT_RUNTIME_REQUIRE_MINGW_DLLS=1)
else()
    add_compile_definitions(TT_RUNTIME_REQUIRE_MINGW_DLLS=0)
endif()

if(TARGET sodium)
    add_compile_definitions(TT_HAS_LIBSODIUM=1)
else()
    message(FATAL_ERROR "Required target missing: sodium")
endif()

if(TARGET zstd::libzstd)
    add_compile_definitions(TT_HAS_ZSTD=1)
else()
    message(FATAL_ERROR "Required target missing: zstd::libzstd")
endif()

set(PLUGIN_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")

# 设置编译器启动器（如ccache）
find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found, enabling compiler launcher.")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
endif()

# --- 2. 包含模块 ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/BuildMetadata.cmake")

# 1. 基础工具
include(TargetSetup)
# include(SourceFileCollection) # REMOVED: Replaced by modular CMakeLists.txt
# include(AddReportsLibrary)      # REMOVED: Moved to src/infrastructure/reports/CMakeLists.txt
# include(AddReportsDataLibrary)  # REMOVED: Moved to src/infrastructure/reports/CMakeLists.txt

# 4. 编译标志
include(BuildTypeFlags)

# --- 3. 添加源码子目录 ---
add_subdirectory(src)

set(ALL_TARGETS)
if(TARGET time_tracer_cli)
    list(APPEND ALL_TARGETS time_tracer_cli)
endif()

# 在每次构建后，复制配置文件
# 在每次构建后，复制配置文件 (Moved to src/CMakeLists.txt)
# foreach(TARGET_NAME ${ALL_TARGETS})
#    ...
# endforeach()

# --- 代码格式化工具 ---
include(AddFormatTarget)
include(AddTidyFixTarget)
# --- 5. 包含特定功能的模块 ---
# include(Win32PostBuildCopy) # Moved to src/CMakeLists.txt 
include(Packaging)        

message(STATUS "CMake configuration finished. Targets are ready to be built.")

# --- START OF FILE CMakeLists.txt ---

# 设置 CMake 所需的最低版本
cmake_minimum_required(VERSION 3.16) # 推荐将版本提升到3.16+以获得最佳的PCH支持

# 定义项目名称和所用语言 (CXX 代表 C++)
project(TimeTrackerApp VERSION "0.2.0" LANGUAGES CXX)

# 设置 C++ 标准为 C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 新增：自动启用 ccache 以加速编译 ---
find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found, enabling compiler launcher.")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
endif()

# --- 为 Release 构建类型设置优化编译选项 ---
# 开发者提示:不要把Os和flto一起使用，这样会导致编译器报错
#本程序的代码没问题.这是一个编译器问题,和本程序没有关系 

# 建议使用Os 本程序为io密集型,开启O3优化效果不明显
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -march=native")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")

# --- 为 Release 构建类型设置优化链接选项 ---
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")


# --- 查找外部依赖 ---
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# --- 收集所有源文件 ---
file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB TIME_MASTER_CLI_SOURCES 
    "time_master_cli/*.cpp"
)
file(GLOB TIME_MASTER_APP_SOURCES 
    "time_master_app/*.cpp"
    "time_master_app/menu_processor/*.cpp"
    "time_master_app/menu_input/*.cpp"
    "time_master_app/*.cpp"
)
file(GLOB DB_INSERTER_SOURCES
    "db_inserter/*.cpp"
    "db_inserter/model/*.cpp"
    "db_inserter/inserter/*.cpp"
    "db_inserter/parser/*.cpp"
    "db_inserter/parser/internal/*.cpp"
)
file(GLOB QUERIES_SOURCES
    "queries/*.cpp"
    "queries/export/*.cpp"
    "queries/daily/*.cpp"
    "queries/daily/formatters/*.cpp"
    "queries/daily/formatters/md/*.cpp"
    "queries/daily/formatters/tex/*.cpp"
    "queries/daily/formatters/typ/*.cpp"

    "queries/monthly/*.cpp"
    "queries/monthly/formatters/*.cpp"
    "queries/monthly/formatters/md/*.cpp"
    "queries/monthly/formatters/tex/*.cpp"
    "queries/monthly/formatters/typ/*.cpp"


    "queries/period/*.cpp"
    "queries/period/formatters/*.cpp"
    "queries/period/formatters/md/*.cpp"
    "queries/period/formatters/tex/*.cpp"
    "queries/period/formatters/typ/*.cpp"


    "queries/shared/*.cpp"
    "queries/shared/breakdown/*.cpp"
    "queries/shared/breakdown/formatters/*.cpp"
)
file(GLOB REPROCESSING_SOURCES

    "reprocessing/*.cpp"

    "reprocessing/Converter/*.cpp"
    "reprocessing/Converter/internal/*.cpp"
    "reprocessing/Converter/model/*.cpp"


    "reprocessing/validator/*.cpp"
    "reprocessing/validator/internal/*.cpp"


)
file(GLOB FILE_HANDLER_SOURCES "file_handler/*.cpp")
file(GLOB ACTION_HANDLER_SOURCES 
    "action_handler/*.cpp"
    "action_handler/database/*.cpp"
    "action_handler/file/*.cpp"
    "action_handler/query/*.cpp"
    "action_handler/reporting/*.cpp"
    

)

# --- 定义两个独立的可执行文件目标 ---
add_executable(
    time_tracker_app          # 可执行文件名
    main.cpp                  # 入口文件
    ${COMMON_SOURCES}
    ${DB_INSERTER_SOURCES}
    ${QUERIES_SOURCES}
    ${TIME_MASTER_APP_SOURCES}
    ${REPROCESSING_SOURCES}
    ${FILE_HANDLER_SOURCES}
    ${ACTION_HANDLER_SOURCES}
)

add_executable(
    time_tracker_cli      # 可执行文件名
    main_cli.cpp          # 入口文件
    ${COMMON_SOURCES}
    ${DB_INSERTER_SOURCES}
    ${QUERIES_SOURCES}
    ${REPROCESSING_SOURCES}
    ${FILE_HANDLER_SOURCES}
    ${ACTION_HANDLER_SOURCES}
    
    ${TIME_MASTER_CLI_SOURCES}
    
)


# --- 为 Windows 平台添加图标资源 ---
if(WIN32)
    find_program(WINDRES_EXECUTABLE windres)
    if(NOT WINDRES_EXECUTABLE)
        message(FATAL_ERROR "Failed to find windres.exe. Please ensure it is in your PATH.")
    endif()
    list(APPEND RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.rc")
    target_sources(time_tracker_app PRIVATE ${RESOURCE_FILES})
    target_sources(time_tracker_cli PRIVATE ${RESOURCE_FILES})
    message(STATUS "Windows icon resource added to both targets.")
endif()

# --- 为每个目标分别添加头文件搜索路径 ---
target_include_directories(time_tracker_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(time_tracker_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# --- 为每个目标分别链接库 ---
target_link_libraries(time_tracker_app PRIVATE
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
    stdc++exp #用于支持 main.cpp中的std::println
)
target_link_libraries(time_tracker_cli PRIVATE
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
    stdc++exp #用于支持 main_cli.cpp中的std::println
)

# --- ★ 新增：直接为最终目标配置预编译头 (路径已修正) ★ ---
target_precompile_headers(time_tracker_app PRIVATE common/pch.h)
target_precompile_headers(time_tracker_cli PRIVATE common/pch.h)


# --- 为每个目标分别设置通用编译选项 ---
target_compile_options(time_tracker_app PRIVATE -Wall )
target_compile_options(time_tracker_cli PRIVATE -Wall )

# ====================================================================
# 安装与打包配置 (Installation and Packaging)
# ====================================================================
option(BUILD_INSTALLER "Build a CPack installer package" OFF)

if(BUILD_INSTALLER)
    install(TARGETS time_tracker_app time_tracker_cli
        RUNTIME DESTINATION bin
    )
    set(UCRT64_BIN_PATH "C:/msys64/ucrt64/bin") # <--- 确保这个路径是正确的
    install(FILES
         "${UCRT64_BIN_PATH}/libsqlite3-0.dll"
        "${UCRT64_BIN_PATH}/libstdc++-6.dll"
        "${UCRT64_BIN_PATH}/libgcc_s_seh-1.dll"
        "${UCRT64_BIN_PATH}/libwinpthread-1.dll"
        DESTINATION bin # 将这些 DLL 也安装到 bin 目录
    )
    install(DIRECTORY config DESTINATION bin)

    # --- 配置 CPack 打包工具 ---
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "一个用于时间管理的工具")
    set(CPACK_PACKAGE_VENDOR "camellia")
    set(CPACK_PACKAGE_CONTACT "https://github.com/camellia2077")
    set(CPACK_GENERATOR "NSIS")
    if(CPACK_GENERATOR STREQUAL "NSIS")
      set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
      set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
      set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    endif()
    include(CPack)
    message(STATUS "CPack packaging is enabled.")
else()
    message(STATUS "CPack packaging is disabled. To enable, use -DBUILD_INSTALLER=ON with cmake.")
endif()

# 打印一条消息，确认配置完成
message(STATUS "CMake configuration finished. Targets 'time_tracker_app' and 'time_tracker_cli' are ready to be built.")
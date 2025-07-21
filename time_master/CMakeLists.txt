# --- START OF FILE CMakeLists.txt ---

# 设置 CMake 所需的最低版本
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和所用语言 (CXX 代表 C++)
project(TimeTrackerApp VERSION "0.2.0" LANGUAGES CXX)

# 设置 C++ 标准为 C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 为 Release 构建类型设置优化编译选项 ---
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")

# --- 为 Release 构建类型设置优化链接选项 ---
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")


# --- 查找外部依赖 ---
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# --- 收集所有源文件 ---
# 使用 file(GLOB ...) 从每个子目录中自动查找源文件
file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB TIME_MASTER_APP_SOURCES "time_master_app/*.cpp")
file(GLOB DB_INSERTER_SOURCES
    "db_inserter/*.cpp"
    "db_inserter/model/*.cpp"
    "db_inserter/inserter/*.cpp"
    "db_inserter/parser/*.cpp"
    "db_inserter/parser/internal/*.cpp"
)
file(GLOB QUERIES_SOURCES
    "queries/*.cpp"
    "queries/report_generators/*.cpp"

    "queries/report_generators/_shared/*.cpp"

    "queries/report_generators/daily/*.cpp"
    "queries/report_generators/daily/formatter/*.cpp"
    "queries/report_generators/daily/formatter/day_md/*.cpp"
    "queries/report_generators/daily/formatter/day_tex/*.cpp"

    "queries/report_generators/daily/querier/*.cpp"

    "queries/report_generators/monthly/*.cpp"
    "queries/report_generators/monthly/formatter/*.cpp"
    "queries/report_generators/monthly/formatter/month_md/*.cpp"
    "queries/report_generators/monthly/formatter/month_tex/*.cpp"
    
    "queries/report_generators/monthly/querier/*.cpp"


    "queries/report_generators/period/*.cpp"
    "queries/report_generators/period/formatter/*.cpp"
    "queries/report_generators/period/formatter/period_md/*.cpp
    "
    "queries/report_generators/period/querier/*.cpp"
)

file(GLOB REPROCESSING_SOURCES
    "reprocessing/*.cpp"
    "reprocessing/validator/*.cpp"
    "reprocessing/validator/internal/*.cpp"
    "reprocessing/input_transfer/*.cpp"
    "reprocessing/input_transfer/internal/*.cpp"

)
file(GLOB FILE_HANDLER_SOURCES "file_handler/*.cpp")
file(GLOB ACTION_HANDLER_SOURCES "action_handler/*.cpp")

# --- 定义两个独立的可执行文件目标 ---

# 目标 1: 交互式菜单应用 (使用 main.cpp)
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

# 目标 2: 命令行工具 (使用 main_cli.cpp)
add_executable(
    time_tracker_cli      # 可执行文件名
    main_cli.cpp          # 入口文件
    ${COMMON_SOURCES}
    ${DB_INSERTER_SOURCES}
    ${QUERIES_SOURCES}
    ${REPROCESSING_SOURCES}
    ${FILE_HANDLER_SOURCES}
    ${ACTION_HANDLER_SOURCES}
)


# --- 为 Windows 平台添加图标资源 ---
if(WIN32)
    # 找到 windres.exe 程序
    find_program(WINDRES_EXECUTABLE windres)
    if(NOT WINDRES_EXECUTABLE)
        message(FATAL_ERROR "Failed to find windres.exe. Please ensure it is in your PATH.")
    endif()

    # 将 .rc 文件添加到源文件列表
    list(APPEND RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.rc")

    # 将资源文件添加到两个目标中
    target_sources(time_tracker_app PRIVATE ${RESOURCE_FILES})
    target_sources(time_tracker_cli PRIVATE ${RESOURCE_FILES})

    # 打印消息
    message(STATUS "Windows icon resource added to both targets.")
endif()

# --- 为每个目标分别添加头文件搜索路径 ---
target_include_directories(time_tracker_app PRIVATE
    "common"

    "db_inserter"
    "queries"
    "resources"
    "time_master_app"

    "reprocessing"

    "file_handler"
    "action_handler"
)
target_include_directories(time_tracker_cli PRIVATE
    "common"

    "db_inserter"

    "queries"

    "resources"

    "reprocessing"

    "file_handler"
    "action_handler"
)

# --- 为每个目标分别链接库 ---
target_link_libraries(time_tracker_app PRIVATE
    SQLite::SQLite3
    nlohmann_json::nlohmann_json)
target_link_libraries(time_tracker_cli PRIVATE
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
    stdc++exp #用于支持 main_cli.cpp中的std::println
    )

# --- 为每个目标分别设置通用编译选项 ---
# -Wextra -Wpedantic
target_compile_options(time_tracker_app PRIVATE -Wall )
target_compile_options(time_tracker_cli PRIVATE -Wall )

# ====================================================================
# 新增内容：安装与打包配置 (Installation and Packaging)
# ====================================================================

# 定义一个选项，用户可以选择是否生成安装包
# 默认关闭 (OFF)，用户可以通过 -DBUILD_INSTALLER=ON 来启用
option(BUILD_INSTALLER "Build a CPack installer package" OFF)

# --- 1. 定义安装规则 ---
# 只有当 BUILD_INSTALLER 为 ON 时才执行安装规则，因为安装规则是 CPack 的基础
if(BUILD_INSTALLER)
    # 安装两个可执行文件
    # DESTINATION bin: 指定将它们安装到安装目录下的 bin 文件夹中
    install(TARGETS time_tracker_app time_tracker_cli
        RUNTIME DESTINATION bin
    )

    # 安装必要的 DLL 文件
    set(UCRT64_BIN_PATH "C:/msys64/ucrt64/bin") # <--- 确保这个路径是正确的

    install(FILES
        "${UCRT64_BIN_PATH}/libsqlite3-0.dll"
        "${UCRT64_BIN_PATH}/libstdc++-6.dll"
        "${UCRT64_BIN_PATH}/libgcc_s_seh-1.dll"
        "${UCRT64_BIN_PATH}/libwinpthread-1.dll"
        DESTINATION bin # 将这些 DLL 也安装到 bin 目录
    )

    # 安装整个 config 目录
    # 这会将源文件中的 'config' 文件夹及其所有内容（如 config.json）
    # 复制到安装目录的 'bin' 文件夹下，最终形成 'bin/config/config.json' 的结构，
    # 这与 C++ 代码的期望完全一致。
    install(DIRECTORY config DESTINATION bin)


    # --- 2. 配置 CPack 打包工具 ---

    # 设置包的通用信息
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "一个用于时间管理的工具")
    set(CPACK_PACKAGE_VENDOR "camellia")
    set(CPACK_PACKAGE_CONTACT "https://github.com/camellia2077")

    # 选择打包生成器
    set(CPACK_GENERATOR "NSIS")

    # 为 NSIS 安装程序设置特定选项
    if(CPACK_GENERATOR STREQUAL "NSIS")
      set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
      set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
      set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
      # 如果您有许可证文件，可以取消下面的注释
      # set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
    endif()

    # 包含 CPack 模块以启用打包功能
    include(CPack)
    message(STATUS "CPack packaging is enabled.")
else()
    message(STATUS "CPack packaging is disabled. To enable, use -DBUILD_INSTALLER=ON with cmake.")
endif()

# 打印一条消息，确认配置完成
message(STATUS "CMake configuration finished. Targets 'time_tracker_app' and 'time_tracker_cli' are ready to be built.")
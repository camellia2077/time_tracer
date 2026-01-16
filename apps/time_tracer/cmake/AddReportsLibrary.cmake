# cmake/AddReportsLibrary.cmake
# 模块：reports_shared 库定义
# 职责：创建 reports_shared 库并配置其属性、宏定义和链接选项。
# 前置依赖：必须在 SourceFileCollection.cmake 之后包含，以获取源文件列表。

# --- 1. 创建库目标 ---
# 将 REPORTS_SHARED_SOURCES 编译为动态库 (SHARED)
add_library(reports_shared SHARED ${REPORTS_SHARED_SOURCES})

# --- 2. 导出符号定义 ---
# 添加此定义以触发源码中的 __declspec(dllexport)
target_compile_definitions(reports_shared PRIVATE REPORTS_SHARED_EXPORTS)

# --- 3. 输出目录配置 ---
# reports_shared 是核心组件，建议将其输出到主程序同级目录 (bin/)
set_target_properties(reports_shared PROPERTIES
    # 将 LIBRARY (.lib/.a) 和 RUNTIME (.dll/.so) 的输出路径都指向 bin 目录
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# --- 4. 应用通用配置 ---
# 应用头文件路径、警告等级、PCH 等通用设置
setup_project_target(reports_shared)

message(STATUS "Library target 'reports_shared' configured.")
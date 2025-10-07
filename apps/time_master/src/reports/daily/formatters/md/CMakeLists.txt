# src/reports/daily/formatters/md/CMakeLists.txt

# --- 1. 定义共享库（DLL）目标 ---
# 使用 ${CMAKE_SOURCE_DIR} 构造从项目根目录开始的绝对路径
add_library(DayMdFormatter SHARED
    DayMd.cpp
    DayMdConfig.cpp
    "${CMAKE_SOURCE_DIR}/src/reports/daily/formatters/base/DayBaseConfig.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/daily/formatters/statistics/StatFormatter.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/shared/utils/format/BoolToString.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/shared/utils/format/TimeFormat.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/shared/utils/format/ReportStringUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/shared/formatters/markdown/MarkdownUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/shared/formatters/base/ProjectTreeFormatter.cpp"
    "${CMAKE_SOURCE_DIR}/src/reports/shared/utils/config/ConfigUtils.cpp"
)

# --- 2. 设置目标属性 ---
# 使用 setup_project_target 函数来应用通用的编译设置
setup_project_target(DayMdFormatter)

# ==================== [核心修改] ====================
# 为 DayMdFormatter 添加最大程度的体积优化选项

# 1. 编译选项：
#    -Os: 优化代码大小
#    -ffunction-sections, -fdata-sections: 将函数和数据放入单独的节，以便链接器进行垃圾回收
target_compile_options(DayMdFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)

# 2. 链接选项：
#    -s: 剥离所有符号表
#    -Wl,--gc-sections: 移除未使用的代码节（需要与 -ffunction-sections 等配合）
target_link_options(DayMdFormatter PRIVATE
    -s
    -Wl,--gc-sections
)
# ====================================================

# --- 3. 设置输出目录 ---
# 使用 LIBRARY_OUTPUT_DIRECTORY 将库文件（.dll, .lib）输出到指定的插件目录
# 这个 PLUGIN_OUTPUT_DIR 变量是在根 CMakeLists.txt 中定义的
set_target_properties(DayMdFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 4. 结束语 ---
message(STATUS "Configured shared library target: DayMdFormatter with maximum size optimization")
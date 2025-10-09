# src/reports/daily/formatters/md/CMakeLists.txt

# --- 1. 定义共享库（DLL）目标 ---
# [核心修改] 源文件列表现在只包含此模块特有的文件。
add_library(DayMdFormatter SHARED
    DayMd.cpp
    DayMdConfig.cpp
)

# --- 2. 设置目标属性 ---
# 使用 setup_project_target 函数来应用通用的编译设置
setup_project_target(DayMdFormatter)

# [新增] 链接到 reports_shared 静态库
# 这会将所有共享的功能（如 StatFormatter, ProjectTreeFormatter 等）链接进来
target_link_libraries(DayMdFormatter PRIVATE reports_shared)

# --- 3. 添加体积优化选项 ---
# 为 DayMdFormatter 添加最大程度的体积优化选项
# 编译选项
target_compile_options(DayMdFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)
# 链接选项
target_link_options(DayMdFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 4. 设置输出目录 ---
# 使用 LIBRARY_OUTPUT_DIRECTORY 将库文件（.dll, .lib）输出到指定的插件目录
# 这个 PLUGIN_OUTPUT_DIR 变量是在根 CMakeLists.txt 中定义的
set_target_properties(DayMdFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 5. 结束语 ---
message(STATUS "Configured shared library target: DayMdFormatter with maximum size optimization")
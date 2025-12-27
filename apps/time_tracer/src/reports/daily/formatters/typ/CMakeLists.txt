# src/reports/daily/formatters/typ/CMakeLists.txt

# --- 1. 定义共享库（DLL）目标 ---
# [核心修改] 源文件列表现在只包含此模块特有的文件。
add_library(DayTypFormatter SHARED
    DayTyp.cpp
    DayTypConfig.cpp
    DayTypUtils.cpp
)

# --- 2. 设置目标属性 ---
setup_project_target(DayTypFormatter)

# [新增] 链接到 reports_shared 静态库
# 这会将所有共享的功能（如 StatFormatter, ProjectTreeFormatter 等）链接进来
target_link_libraries(DayTypFormatter PRIVATE reports_shared)

# --- 3. 添加体积优化选项 ---
target_compile_options(DayTypFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)
target_link_options(DayTypFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 4. 设置输出目录 ---
set_target_properties(DayTypFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 5. 结束语 ---
message(STATUS "Configured shared library target: DayTypFormatter with maximum size optimization")
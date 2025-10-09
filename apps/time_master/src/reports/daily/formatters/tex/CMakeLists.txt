# src/reports/daily/formatters/tex/CMakeLists.txt

# --- 1. 定义共享库（DLL）目标 ---
# 源文件列表只包含此模块特有的文件。
add_library(DayTexFormatter SHARED
    DayTex.cpp
    DayTexConfig.cpp
    DayTexUtils.cpp
)

# --- 2. 设置目标属性 ---
# 使用 setup_project_target 函数来应用通用的编译设置
setup_project_target(DayTexFormatter)

# --- 3. 链接到共享库 ---
# 链接到 reports_shared 静态库，获取所有共享功能
target_link_libraries(DayTexFormatter PRIVATE reports_shared)

# --- 4. 添加体积优化选项 ---
# 为 DayTexFormatter 添加最大程度的体积优化选项
target_compile_options(DayTexFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)
target_link_options(DayTexFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 5. 设置输出目录 ---
# 将生成的 DLL 文件输出到 build/bin/plugins 目录
set_target_properties(DayTexFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 6. 结束语 ---
message(STATUS "Configured shared library target: DayTexFormatter with maximum size optimization")
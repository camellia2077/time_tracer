# src/reports/monthly/formatters/tex/CMakeLists.txt

# --- 1. 定义共享库（DLL）目标 ---
add_library(MonthTexFormatter SHARED
    MonthTex.cpp
    MonthTexConfig.cpp
    MonthTexUtils.cpp
)

# --- 2. 设置目标属性 ---
setup_project_target(MonthTexFormatter)

# --- 3. 链接到共享库 ---
target_link_libraries(MonthTexFormatter PRIVATE reports_shared)

# --- 4. 添加体积优化选项 ---
target_compile_options(MonthTexFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)
target_link_options(MonthTexFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 5. 设置输出目录 ---
set_target_properties(MonthTexFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 6. 结束语 ---
message(STATUS "Configured shared library target: MonthTexFormatter with maximum size optimization")
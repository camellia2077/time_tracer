# src/reports/period/formatters/typ/CMakeLists.txt

# --- 1. 定义共享库（DLL）目标 ---
add_library(PeriodTypFormatter SHARED
    PeriodTyp.cpp
    PeriodTypConfig.cpp
)

# --- 2. 设置目标属性 ---
setup_project_target(PeriodTypFormatter)

# --- 3. 链接到共享库 ---
target_link_libraries(PeriodTypFormatter PRIVATE reports_shared)

# --- 4. 添加体积优化选项 ---
target_compile_options(PeriodTypFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)
target_link_options(PeriodTypFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 5. 设置输出目录 ---
set_target_properties(PeriodTypFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 6. 结束语 ---
message(STATUS "Configured shared library target: PeriodTypFormatter with maximum size optimization")
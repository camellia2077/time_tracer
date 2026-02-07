# src/reports/range/formatters/markdown/CMakeLists.txt

# --- 1. 定义共享库(DLL)目标 ---
add_library(RangeMdFormatter SHARED
    range_md_formatter.cpp
    range_md_config.cpp
)

# --- 2. 设置目标属性 ---
setup_project_target(RangeMdFormatter)

# --- 3. 链接到共享库 ---
target_link_libraries(RangeMdFormatter PRIVATE reports_shared)

# --- 4. 添加体积优化选项 ---
target_compile_options(RangeMdFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)

target_link_options(RangeMdFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 5. 设置输出目录 ---
set_target_properties(RangeMdFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 6. 结束语 ---
message(STATUS "Configured shared library target: RangeMdFormatter with maximum size optimization")

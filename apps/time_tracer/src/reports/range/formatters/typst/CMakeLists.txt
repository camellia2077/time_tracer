# src/reports/range/formatters/typst/CMakeLists.txt

# --- 1. 定义共享库(DLL)目标 ---
add_library(RangeTypFormatter SHARED
    range_typ_formatter.cpp
    range_typ_config.cpp
)

# --- 2. 设置目标属性 ---
setup_project_target(RangeTypFormatter)

# --- 3. 链接到共享库 ---
target_link_libraries(RangeTypFormatter PRIVATE reports_shared)

# --- 4. 添加体积优化选项 ---
target_compile_options(RangeTypFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)

target_link_options(RangeTypFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 5. 设置输出目录 ---
set_target_properties(RangeTypFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 6. 结束语 ---
message(STATUS "Configured shared library target: RangeTypFormatter with maximum size optimization")

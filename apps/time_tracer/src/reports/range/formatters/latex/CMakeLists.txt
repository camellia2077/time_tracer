# src/reports/range/formatters/latex/CMakeLists.txt

# --- 1. 定义共享库(DLL)目标 ---
add_library(RangeTexFormatter SHARED
    range_tex_formatter.cpp
    range_tex_config.cpp
    range_tex_utils.cpp
)

# --- 2. 设置目标属性 ---
setup_project_target(RangeTexFormatter)

# --- 3. 链接到共享库 ---
target_link_libraries(RangeTexFormatter PRIVATE reports_shared)

# --- 4. 添加体积优化选项 ---
target_compile_options(RangeTexFormatter PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)

target_link_options(RangeTexFormatter PRIVATE
    -s
    -Wl,--gc-sections
)

# --- 5. 设置输出目录 ---
set_target_properties(RangeTexFormatter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# --- 6. 结束语 ---
message(STATUS "Configured shared library target: RangeTexFormatter with maximum size optimization")

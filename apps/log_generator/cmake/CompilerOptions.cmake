# 负责编译器优化和警告设置
# --- Optimization and Compilation Options ---
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(log_generator PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    -Wall -Wextra # [建议] 顺便加上基础警告
  )
endif()

# 启用链接时优化 (LTO/IPO)
set_property(TARGET log_generator PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET log_generator PROPERTY LINK_FLAGS_RELEASE "-s")

# --- [新增] Precompiled Headers (PCH) 配置 ---
# 指定 PCH 头文件路径
target_precompile_headers(log_generator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

message(STATUS "Precompiled Headers enabled: src/pch.hpp")
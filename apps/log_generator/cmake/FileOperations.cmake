
# 负责配置文件复制等辅助任务
# --- Build-time Configuration File Copying ---

# 定义需要复制的文件
set(CONFIG_FILES 
    "activities_config.toml"
    "mapping_config.toml"
)

# 确保输出目录存在
add_custom_command(
    TARGET log_generator PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/config
    COMMENT "Ensuring config directory exists in build directory"
)

set(CONFIG_DEST_FILES "")

# 遍历并创建复制命令
foreach(FILENAME ${CONFIG_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/config/${FILENAME}")
    set(DEST "${CMAKE_BINARY_DIR}/config/${FILENAME}")
    
    add_custom_command(
        OUTPUT ${DEST}
        COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DEST}
        DEPENDS ${SRC}
        COMMENT "Copying ${FILENAME} to build directory"
    )
    list(APPEND CONFIG_DEST_FILES ${DEST})
endforeach()

# 创建自定义目标以触发复制
add_custom_target(copy_configs ALL DEPENDS ${CONFIG_DEST_FILES})
add_dependencies(log_generator copy_configs)
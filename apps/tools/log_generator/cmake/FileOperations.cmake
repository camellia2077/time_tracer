# Build-time output/copy rules:
# 1) Place executable under build/bin
# 2) Copy config files under build/bin/config

set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(CONFIG_DEST_DIR "${APP_BIN_DIR}/config")

set_target_properties(log_generator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${APP_BIN_DIR}"
)

set(CONFIG_FILES
    "activities_config.toml"
    "alias_mapping.toml"
)

set(CONFIG_DEST_FILES "")

foreach(FILENAME ${CONFIG_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/config/${FILENAME}")
    set(DEST "${CONFIG_DEST_DIR}/${FILENAME}")

    add_custom_command(
        OUTPUT "${DEST}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CONFIG_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC}" "${DEST}"
        DEPENDS "${SRC}"
        COMMENT "Copying ${FILENAME} to ${CONFIG_DEST_DIR}"
        VERBATIM
    )
    list(APPEND CONFIG_DEST_FILES "${DEST}")
endforeach()

add_custom_target(copy_configs ALL DEPENDS ${CONFIG_DEST_FILES})
add_dependencies(log_generator copy_configs)

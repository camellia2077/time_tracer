# Build-time output/copy rules:
# 1) Place executable under build/bin
# 2) Copy config files under build/bin/config

set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(CONFIG_DEST_DIR "${APP_BIN_DIR}/config")
set(CONVERTER_DEST_DIR "${CONFIG_DEST_DIR}/converter")
set(CONVERTER_ALIASES_DEST_DIR "${CONVERTER_DEST_DIR}/aliases")

set_target_properties(log_generator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${APP_BIN_DIR}"
)

set(APP_CONFIG_FILES
    "activities_config.toml"
)

set(CONFIG_DEST_FILES "")

foreach(FILENAME ${APP_CONFIG_FILES})
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

set(CONVERTER_CONFIG_FILES
    "alias_mapping.toml"
)

foreach(FILENAME ${CONVERTER_CONFIG_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/tracer_core/config/converter/${FILENAME}")
    set(DEST "${CONVERTER_DEST_DIR}/${FILENAME}")

    add_custom_command(
        OUTPUT "${DEST}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CONVERTER_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC}" "${DEST}"
        DEPENDS "${SRC}"
        COMMENT "Copying converter ${FILENAME} to ${CONVERTER_DEST_DIR}"
        VERBATIM
    )
    list(APPEND CONFIG_DEST_FILES "${DEST}")
endforeach()

set(CONVERTER_ALIAS_FILES
    "meal.toml"
    "recreation.toml"
    "routine.toml"
    "sleep.toml"
    "rest.toml"
    "exercise.toml"
    "study.toml"
)

foreach(FILENAME ${CONVERTER_ALIAS_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/tracer_core/config/converter/aliases/${FILENAME}")
    set(DEST "${CONVERTER_ALIASES_DEST_DIR}/${FILENAME}")

    add_custom_command(
        OUTPUT "${DEST}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CONVERTER_ALIASES_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC}" "${DEST}"
        DEPENDS "${SRC}"
        COMMENT "Copying converter alias ${FILENAME} to ${CONVERTER_ALIASES_DEST_DIR}"
        VERBATIM
    )
    list(APPEND CONFIG_DEST_FILES "${DEST}")
endforeach()

add_custom_target(copy_configs ALL DEPENDS ${CONFIG_DEST_FILES})
add_dependencies(log_generator copy_configs)

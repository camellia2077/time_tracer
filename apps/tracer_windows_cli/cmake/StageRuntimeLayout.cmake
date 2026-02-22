if(NOT DEFINED SOURCE_BIN OR SOURCE_BIN STREQUAL "")
    message(FATAL_ERROR "StageRuntimeLayout: SOURCE_BIN is required.")
endif()
if(NOT DEFINED DEST_BIN OR DEST_BIN STREQUAL "")
    message(FATAL_ERROR "StageRuntimeLayout: DEST_BIN is required.")
endif()

file(MAKE_DIRECTORY "${DEST_BIN}")

set(PRIMARY_FILES
    "time_tracer_cli.exe"
    "time_tracer_core.dll"
)

foreach(FILE_NAME IN LISTS PRIMARY_FILES)
    set(SRC_PATH "${SOURCE_BIN}/${FILE_NAME}")
    if(NOT EXISTS "${SRC_PATH}")
        message(FATAL_ERROR
            "StageRuntimeLayout: required file not found: ${SRC_PATH}"
        )
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${SRC_PATH}" "${DEST_BIN}/"
        COMMAND_ERROR_IS_FATAL ANY
    )
endforeach()

file(GLOB RUNTIME_DLLS "${SOURCE_BIN}/*.dll")
foreach(DLL_PATH IN LISTS RUNTIME_DLLS)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${DLL_PATH}" "${DEST_BIN}/"
        COMMAND_ERROR_IS_FATAL ANY
    )
endforeach()

if(EXISTS "${SOURCE_BIN}/config")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${SOURCE_BIN}/config" "${DEST_BIN}/config"
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

if(EXISTS "${SOURCE_BIN}/plugins")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${SOURCE_BIN}/plugins" "${DEST_BIN}/plugins"
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

file(GLOB_RECURSE STAGED_ITEMS RELATIVE "${DEST_BIN}" "${DEST_BIN}/*")
list(SORT STAGED_ITEMS)
string(JOIN "\n" MANIFEST_CONTENT ${STAGED_ITEMS})
file(WRITE "${DEST_BIN}/runtime_manifest.txt" "${MANIFEST_CONTENT}\n")

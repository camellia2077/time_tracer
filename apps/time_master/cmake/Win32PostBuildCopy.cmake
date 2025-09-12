# 模块3：DLL复制逻辑 (Win32PostBuildCopy.cmake)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Setting up post-build DLL copy for MinGW.")
    
    get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    set(REQUIRED_DLLS
        "libgcc_s_seh-1.dll"
        "libstdc++-6.dll"
        "libsqlite3-0.dll"
        "libwinpthread-1.dll"
    )

    # 注意：这里我们假设所有可执行文件目标都被添加到了 ALL_TARGETS 变量中
    foreach(TARGET_NAME ${ALL_TARGETS})
        foreach(DLL_NAME ${REQUIRED_DLLS})
            set(DLL_PATH "${MINGW_BIN_DIR}/${DLL_NAME}")
            if(EXISTS ${DLL_PATH})
                add_custom_command(
                    TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL_PATH}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                    COMMENT "Copying ${DLL_NAME} for ${TARGET_NAME}"
                )
            else()
                message(WARNING "Could not find required DLL: ${DLL_PATH}")
            endif()
        endforeach()
    endforeach()
endif()
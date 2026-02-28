# 模块3：DLL复制逻辑 (Win32PostBuildCopy.cmake)

if(WIN32)
    message(STATUS "Setting up post-build DLL copy.")

    set(REQUIRED_DLLS)
    if(NOT TT_USE_BUNDLED_SQLITE)
        list(APPEND REQUIRED_DLLS "libsqlite3-0.dll")
    endif()
    if(NOT TT_TOML_HEADER_ONLY)
        list(APPEND REQUIRED_DLLS "libtomlplusplus-3.dll")
    endif()

    if(MINGW AND NOT TT_STATIC_MINGW_RUNTIME)
        list(APPEND REQUIRED_DLLS
            "libgcc_s_seh-1.dll"
            "libstdc++-6.dll"
            "libwinpthread-1.dll"
        )
    endif()

    if(NOT REQUIRED_DLLS)
        message(STATUS "No external runtime DLLs selected for post-build copy.")
        return()
    endif()

    get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    foreach(TARGET_NAME ${ALL_TARGETS})
        foreach(DLL_NAME ${REQUIRED_DLLS})
            set(DLL_PATH "${COMPILER_BIN_DIR}/${DLL_NAME}")
            if(EXISTS "${DLL_PATH}")
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

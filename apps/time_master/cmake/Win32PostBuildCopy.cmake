# 模块3：DLL复制逻辑 (Win32PostBuildCopy.cmake)

if(WIN32)
    message(STATUS "Setting up post-build DLL copy.")
    
    # 定义一个变量来存储需要复制的DLL列表
    set(REQUIRED_DLLS
        "libsqlite3-0.dll" # SQLite3 是一个外部依赖
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "GCC detected, adding GCC runtime DLLs.")
        # GCC/MinGW 运行时的 DLL
        list(APPEND REQUIRED_DLLS
            "libgcc_s_seh-1.dll"
            "libstdc++-6.dll"
            "libwinpthread-1.dll"
        )
        
        # 查找 GCC/MinGW 的 bin 目录
        get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Clang detected, adding Clang runtime DLLs.")
        # Clang 运行时的 DLL，通常位于 LLVM 安装目录
        # 这里的路径需要根据实际情况调整，通常在 LLVM/bin 目录下
        list(APPEND REQUIRED_DLLS
            "libwinpthread-1.dll"
        )
        # TODO: 根据你的 Clang 安装路径来设置 COMPILER_BIN_DIR
        # 例如：set(COMPILER_BIN_DIR "C:/Program Files/LLVM/bin")
        get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    else()
        message(WARNING "Unknown compiler ID: ${CMAKE_CXX_COMPILER_ID}. DLL copy might fail.")
    endif()

    # 复制DLL
    foreach(TARGET_NAME ${ALL_TARGETS})
        foreach(DLL_NAME ${REQUIRED_DLLS})
            set(DLL_PATH "${COMPILER_BIN_DIR}/${DLL_NAME}")
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
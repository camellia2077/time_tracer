# 模块4：打包配置 (Packaging.cmake)

option(BUILD_INSTALLER "Build a CPack installer package" OFF)

if(BUILD_INSTALLER)
    # 安装主程序到 bin 目录
    install(TARGETS ${ALL_TARGETS}
        RUNTIME DESTINATION bin
    )

    # --- [核心修改] 安装所有 DLL 到 bin/plugins 目录 ---
    install(TARGETS 
        DayMdFormatter
        DayTypFormatter
        DayTexFormatter     # [新增]
        MonthMdFormatter    # [新增]
        RUNTIME DESTINATION bin/plugins
        LIBRARY DESTINATION bin/plugins
    )
    # ---------------------------------------------------

    set(UCRT64_BIN_PATH "C:/msys64/ucrt64/bin") # 请确保msys64保这个路径是正确的
    # 安装依赖的运行时 DLL 到 bin 目录
    install(FILES
        "${UCRT64_BIN_PATH}/libsqlite3-0.dll"
        "${UCRT64_BIN_PATH}/libstdc++-6.dll"
        "${UCRT64_BIN_PATH}/libgcc_s_seh-1.dll"
        "${UCRT64_BIN_PATH}/libwinpthread-1.dll"
        DESTINATION bin
    )
    # 安装配置文件
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/config" DESTINATION .)

    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    
    # 将描述改为纯英文，以避免CPack在生成安装包脚本时出现编码问题
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool for time management.")

    set(CPACK_PACKAGE_VENDOR "camellia")
    set(CPACK_PACKAGE_CONTACT "https://github.com/camellia2077")
    set(CPACK_GENERATOR "NSIS")

    if(CPACK_GENERATOR STREQUAL "NSIS")
      set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
      set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
      set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    endif()

    include(CPack)
    message(STATUS "CPack packaging is enabled.")
else()
    message(STATUS "CPack packaging is disabled. To enable, use -DBUILD_INSTALLER=ON with cmake.")
endif()
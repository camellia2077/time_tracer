# 模块4：打包配置 (Packaging.cmake)

option(BUILD_INSTALLER "Build a CPack installer package" OFF)

if(BUILD_INSTALLER)
    install(TARGETS ${ALL_TARGETS}
        RUNTIME DESTINATION bin
    )
    
    set(UCRT64_BIN_PATH "C:/msys64/ucrt64/bin") # <--- 确保这个路径是正确的
    install(FILES
        "${UCRT64_BIN_PATH}/libsqlite3-0.dll"
        "${UCRT64_BIN_PATH}/libstdc++-6.dll"
        "${UCRT64_BIN_PATH}/libgcc_s_seh-1.dll"
        "${UCRT64_BIN_PATH}/libwinpthread-1.dll"
        DESTINATION bin
    )
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/config" DESTINATION .)

    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "一个用于时间管理的工具")
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
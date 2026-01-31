# cmake/AddFormatTarget.cmake

# 1. 查找工具
find_program(CLANG_FORMAT_EXE NAMES "clang-format")

if(CLANG_FORMAT_EXE)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXE}")

    # 2. 自动扫描所有头文件
    # 逻辑修补：原配置只包含了 .cpp 文件，这里补充扫描 src 目录下所有的头文件
    file(GLOB_RECURSE ALL_HEADERS
        "src/*.hpp"
        "src/*.h"
        # 如果有单独的 include 目录，也可以在这里添加，例如: "include/*.hpp"
    )

    # 3. 汇总所有需要格式化的文件
    # 注意：这里结合了自动扫描的头文件和 SourceFileCollection.cmake 中定义的源文件
    set(ALL_FORMAT_SOURCES
        ${ALL_HEADERS}            # [新增] 包含头文件
        ${CORE_SOURCES}
        ${COMMON_SOURCES}
        ${SERIALIZER_SOURCES}
        ${VALIDATOR_SOURCES}
        ${BOOTSTRAP_SOURCES}
        ${CONFIG_SOURCES}
        ${IMPORTER_SOURCES}
        ${REPORTS_SOURCES}
        ${CONVERTER_SOURCES}
        ${IO_SOURCES}
        ${CLI_SOURCES}
        ${REPORTS_DATA_SOURCES}
        ${REPORTS_SHARED_SOURCES}
        "src/main_cli.cpp"
    )

    # 4. 定义自定义目标 'format'
    # 使用 -style=file 让 clang-format 读取项目根目录下的 .clang-format 配置文件
    # 逻辑优化：添加 WORKING_DIRECTORY 确保 .clang-format 能被正确找到
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format on all source (.cpp) and header (.hpp) files..."
        VERBATIM
    )
else()
    message(WARNING "clang-format not found. 'format' target will not be available.")
endif()
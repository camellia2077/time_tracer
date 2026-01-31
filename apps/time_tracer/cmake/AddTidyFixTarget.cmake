# cmake/AddTidyFixTarget.cmake

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy for fix: ${CLANG_TIDY_EXE}")

    # 1. 汇总源文件 (直接复用 SourceFileCollection.cmake 中的变量)
    # 注意：我们只对 .cpp 进行 Tidy 检查，因为头文件会被包含在 .cpp 中一同分析。
    # 如果单独检查 .hpp，往往因为缺乏上下文而报错。
    set(ALL_TIDY_SOURCES
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

    # 2. 创建顶层目标
    add_custom_target(tidy-fix)

    # 3. 构建串行任务链
    # 我们将为每个文件创建一个独立的 Target，并强制它们按顺序执行。
    # 这样做的目的是：
    # A. 让 Ninja 显示进度条 [x/N]
    # B. 避免并行修改头文件导致的文件损坏 (Race Condition)
    
    set(PREV_TARGET "")
    set(COUNTER 0)
    
    foreach(FILE_PATH ${ALL_TIDY_SOURCES})
        math(EXPR COUNTER "${COUNTER} + 1")
        
        # 生成唯一的目标名称 (使用 Counter 避免重名冲突)
        set(CURRENT_TARGET "tidy_step_${COUNTER}")

        # 定义单文件任务
        add_custom_target(${CURRENT_TARGET}
            COMMAND ${CLANG_TIDY_EXE} 
                -p ${CMAKE_BINARY_DIR} 
                --fix 
                --format-style=file 
                "${CMAKE_SOURCE_DIR}/${FILE_PATH}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "[${COUNTER}] Analyzing and Fixing: ${FILE_PATH}"
            VERBATIM
        )

        # 建立依赖链：当前任务依赖上一个任务
        # 结果：T1 -> T2 -> T3 ...
        if(PREV_TARGET)
            add_dependencies(${CURRENT_TARGET} ${PREV_TARGET})
        endif()

        # 更新前驱指针
        set(PREV_TARGET ${CURRENT_TARGET})
    endforeach()

    # 4. 让顶层目标依赖链条的最后一个环节，从而触发整条链
    if(PREV_TARGET)
        add_dependencies(tidy-fix ${PREV_TARGET})
        message(STATUS "Configured tidy-fix chain for ${COUNTER} files.")
    endif()

else()
    message(WARNING "clang-tidy not found. 'tidy-fix' target will not be available.")
endif()
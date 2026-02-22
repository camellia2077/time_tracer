# clang-tidy support

option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" ON)

if(ENABLE_CLANG_TIDY)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found. Static analysis will be disabled.")
    endif()
endif()

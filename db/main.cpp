#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <algorithm>
#include <iomanip> // For std::setprecision and std::fixed
#include <sqlite3.h>
#include <filesystem>
#include <chrono>

#include "common_utils.h"
#include "data_parser.h"
#include "database_inserter.h"
#include "query_handler.h" // Main query handler header

#include "menu.h"
#include "processing.h"

// 定义命名空间别名
namespace fs = std::filesystem;

// 核心常量定义
const std::string DATABASE_NAME = "time_data.db";

/**
 * @brief 用于处理文件处理工作流
 *
 * 此函数现在调用在 processing 模块中定义的同名函数。
 * 它由 Menu 类调用。
 */
void handle_process_files_wrapper() { // Renamed to avoid conflicts if headers are included in a different order
    // 调用位于 processing 模块中的函数，并传递全局数据库名称
    handle_process_files(DATABASE_NAME);
}


// --- The Menu class and open_database_if_needed() function have been removed ---
// (Assuming the Menu class is now in Menu/Menu.h and Menu/Menu.cpp)


#if defined(_WIN32) || defined(_WIN64)
void EnableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

int main() {
    #if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    EnableVirtualTerminalProcessing();
    #endif

    // 实例化菜单，将数据库名称传递给其构造函数
    Menu app_menu(DATABASE_NAME);
    app_menu.run();
    
    return 0;
}
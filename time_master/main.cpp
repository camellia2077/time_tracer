#include "common/pch.h"
// main.cpp
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <string>

#include "time_master_app/menu.h" 
#include "common/common_utils.h"     // Contains color macro definitions
#include "file_handler/FileController.h"   // 包含json读取和递归查询文件

// Core constant for the database name
const std::string DATABASE_NAME = "time_data.db";

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

int main(int argc, char* argv[]) {
    #if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    EnableVirtualTerminalProcessing();
    #endif

    if (argc < 1) {
        std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": Cannot determine application path." << std::endl;
        return 1;
    }

    try {
        // --- 新的、简化的初始化流程 ---

        // 1. 实例化 FileController。这一个步骤就完成了所有配置加载。
        //    如果加载失败，其构造函数会抛出异常，由下面的 catch 块捕获。
        FileController file_controller(argv[0]);
        
        // 2. 实例化并运行菜单。我们直接从控制器获取所需的数据来初始化它。
        Menu app_menu(
            DATABASE_NAME,
            file_controller.get_config(),          // 从控制器获取 AppConfig
            file_controller.get_main_config_path() // 从控制器获取主配置路径
        );
        app_menu.run();

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Fatal Error" << RESET_COLOR << " during configuration setup: " << e.what() << std::endl;
        // 在交互模式下，可以增加一个暂停，让用户看到错误信息
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    return 0;
}
// src/main_cli.cpp
#include <iostream> 
#include <print>
#include <string>
#include <vector>
#include <stdexcept>

// --- Windows-specific include for console functions ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "common/AnsiColors.hpp"
#include "common/version.hpp"
#include "time_master_cli/CliController.hpp"
// [新增] 引入帮助模块头文件
#include "time_master_cli/CliHelp.hpp"

int main(int argc, char* argv[]) {
    // --- Console Setup (Windows Only) ---
    #if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    #endif

    std::vector<std::string> args(argv, argv + argc);
    
    // 如果没有参数，打印帮助
    if (args.size() < 2) {
        print_full_usage(args[0].c_str());
        return 1;
    }

    // 处理缩写命令
    if (args[1] == "pre") {
        args[1] = "preprocess";
    }

    const std::string& command = args[1];

    // 处理全局 help/version 参数
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "-v" || command == "--version") {
        std::println("TimeMaster Command Version: {}", AppInfo::VERSION);
        std::println("Last Updated:  {}", AppInfo::LAST_UPDATED);
        return 0;
    }

    // 将复杂命令委托给控制器
    try {
        CliController controller(args);
        controller.execute();
    } catch (const std::exception& e) {
        std::println(std::cerr, "{}{}{}{}", RED_COLOR, "Error: ", RESET_COLOR, e.what());
        
        if (std::string(e.what()).find("command") != std::string::npos || 
            std::string(e.what()).find("argument") != std::string::npos) {
             std::println("\nUse '{}' for more information.", args[0]);
        }
        return 1;
    }

    return 0;
}
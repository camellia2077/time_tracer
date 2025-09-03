// main.cpp
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <print>
#include <string>
#include <filesystem> // 引入 filesystem

#include "time_master_app/menu.hpp" 
#include "common/common_utils.hpp"
#include "file_handler/FileController.hpp"

namespace fs = std::filesystem;

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
        std::println(std::cerr, "{}{}{}{}", RED_COLOR, "Error", RESET_COLOR, ": Cannot determine application path.");
        return 1;
    }

    try {
        // --- 路径初始化 ---
        fs::path output_root_path;
        fs::path exported_files_path;

        std::cout << "Enter the path for exported reports (e.g., .md, .tex) or press Enter for default:\n> ";
        std::string user_path;
        std::getline(std::cin, user_path);

        if (!user_path.empty()) {
            // 用户指定了路径, 该路径为报告专用路径
            exported_files_path = fs::absolute(user_path);
            // 主输出目录是其父目录
            output_root_path = exported_files_path.parent_path();
        } else {
            // 默认情况：主输出目录是程序所在位置下的 "output"
            fs::path exe_path(argv[0]);
            output_root_path = fs::absolute(exe_path.parent_path() / "output");
            // 报告专用路径是主目录下的 "exported_files"
            exported_files_path = output_root_path / "exported_files";
        }

        // 确保目录存在
        fs::create_directories(output_root_path);
        fs::create_directories(exported_files_path);

        std::cout << "\n" << "All program outputs will be saved under: " << output_root_path << std::endl;
        std::cout << "Exported reports (.md, .tex, etc.) will be saved in: " << exported_files_path << std::endl;
        
        // --- 程序初始化 ---
        FileController file_controller(argv[0]);
        
        // 实例化并运行菜单，传入已确定的路径
        Menu app_menu(
            file_controller.get_config(),
            file_controller.get_main_config_path(),
            output_root_path,
            exported_files_path
        );
        app_menu.run();

    } catch (const std::exception& e) {
        std::println(std::cerr, "{}{}{} during configuration setup: {}", RED_COLOR, "Fatal Error", RESET_COLOR, e.what());
        std::print("\nPress Enter to exit...");
        std::cin.get();
        return 1;
    }
    
    return 0;
}
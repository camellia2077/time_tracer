#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <filesystem>

#include "Menu/menu.h" // 根据你的目录结构修改

// 定义命名空间别名
namespace fs = std::filesystem;

// 核心常量定义
const std::string DATABASE_NAME = "time_data.db";
const std::string CONFIG_FILE_NAME = "config.json";
const std::string CONFIG_DIR_NAME = "config"; // 【新增】定义配置目录名称

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

    // --- 【核心修改】确定配置文件路径 ---
    std::string config_path;
    try {
        fs::path exe_path = fs::canonical(fs::path(argv[0])).parent_path();
        
        // 【核心修改】构建指向 "config" 子目录中配置文件的路径
        // 旧路径: exe_path / CONFIG_FILE_NAME
        // 新路径: exe_path / CONFIG_DIR_NAME / CONFIG_FILE_NAME
        config_path = (exe_path / CONFIG_DIR_NAME / CONFIG_FILE_NAME).string();

        if (!fs::exists(config_path)) {
            // 【修改】更新警告消息以反映新的位置
            std::cerr << "Warning: Configuration file '" << CONFIG_FILE_NAME 
                      << "' not found in the '" << CONFIG_DIR_NAME 
                      << "' subdirectory." << std::endl;
            std::cerr << "Expected at: " << config_path << std::endl;
            // 即使文件不存在，也继续运行，但后续操作会收到警告
        }
    } catch (const std::exception& e) {
        std::cerr << "Error determining config file path: " << e.what() << std::endl;
        config_path = ""; // 路径无效，设置为空
    }

    // --- 【核心修改】实例化菜单，将数据库名称和配置文件路径传递给其构造函数 ---
    Menu app_menu(DATABASE_NAME, config_path);
    app_menu.run();
    
    return 0;
}
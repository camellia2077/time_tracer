// --- START OF FILE main.cpp --- (FINAL CORRECTED VERSION)

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

#include "menu.h" 
#include "LogProcessor.h"     // 包含了 AppConfig 的定义
#include <nlohmann/json.hpp>   
#include "common_utils.h"     // 包含了颜色宏的定义

// 定义命名空间别名
namespace fs = std::filesystem;

// 核心常量定义
const std::string DATABASE_NAME = "time_data.db";
const std::string CONFIG_FILE_NAME = "config.json";
const std::string CONFIG_DIR_NAME = "config"; 

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

    AppConfig app_config; 
    std::string main_config_path_str; // 用于存储 config.json 的绝对路径

    try {
        // 1. 获取可执行文件所在的目录
        fs::path exe_path = fs::canonical(fs::path(argv[0])).parent_path();
        
        // 2. 构建到 config 目录的路径
        fs::path config_dir_path = exe_path / CONFIG_DIR_NAME;
        
        // 3. 构建到主配置文件 config.json 的完整路径
        fs::path main_config_path = config_dir_path / CONFIG_FILE_NAME;
        main_config_path_str = main_config_path.string(); // 保存这个路径，以便后续传递

        // 4. 检查主配置文件是否存在
        if (!fs::exists(main_config_path)) {
            std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": Configuration file '" << CONFIG_FILE_NAME 
                      << "' not found in the '" << CONFIG_DIR_NAME 
                      << "' subdirectory." << std::endl;
            // 打印出程序期望找到的正确路径，方便调试
            std::cerr << "Expected at: " << main_config_path.string() << std::endl;
            return 1;
        }

        // 5. 解析主配置文件
        std::ifstream config_file(main_config_path);
        nlohmann::json j;
        config_file >> j;
        
        // 6. 从 JSON 读取【相对】路径，并与【config目录】拼接，形成其他配置文件的【绝对】路径
        std::string ip_config_relative = j.at("interval_processor_config_path").get<std::string>();
        app_config.interval_processor_config_path = (config_dir_path / ip_config_relative).string();

        std::string fv_config_relative = j.at("format_validator_config_path").get<std::string>();
        app_config.format_validator_config_path = (config_dir_path / fv_config_relative).string();

        std::string error_log_relative = j.at("error_log_path").get<std::string>();
        // 使用 lexically_normal() 来正确处理 ".." 这样的相对路径
        app_config.error_log_path = (config_dir_path / error_log_relative).lexically_normal().string();

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR <<  "Fatal Error "<< RESET_COLOR <<" during configuration setup: " << e.what() << std::endl;
        return 1;
    }

    // 7. 实例化菜单，将数据库名称、填充好的 AppConfig 和主配置文件的绝对路径传递给构造函数
    Menu app_menu(DATABASE_NAME, app_config, main_config_path_str);
    app_menu.run();
    
    return 0;
}
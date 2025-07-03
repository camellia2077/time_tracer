/*
████████╗██╗███╗   ███╗███████╗    ███╗   ███╗ █████╗ ███████╗████████╗███████╗██████╗ 
╚══██╔══╝██║████╗ ████║██╔════╝    ████╗ ████║██╔══██╗██╔════╝╚══██╔══╝██╔════╝██╔══██╗
   ██║   ██║██╔████╔██║█████╗      ██╔████╔██║███████║███████╗   ██║   █████╗  ██████╔╝
   ██║   ██║██║╚██╔╝██║██╔══╝      ██║╚██╔╝██║██╔══██║╚════██║   ██║   ██╔══╝  ██╔══██╗
   ██║   ██║██║ ╚═╝ ██║███████╗    ██║ ╚═╝ ██║██║  ██║███████║   ██║   ███████╗██║  ██║
   ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝
                                                                                                                                                                                                           
*/
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include <sqlite3.h>

// --- Platform-specific includes for console ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// --- Project-specific includes ---
#include "common_utils.h"
#include "version.h"
#include "LogProcessor.h"
#include "processing.h"
#include "query_handler.h"
#include <nlohmann/json.hpp>

#include "common_utils.h" // 调用颜色
/*
#define RESET_COLOR   "\033[0m"
#define RED_COLOR     "\033[31m"
#define GREEN_COLOR   "\033[32m"
#define YELLOW_COLOR  "\033[33m"
*/

// --- Namespace alias ---
namespace fs = std::filesystem;

// --- 全局常量 ---
const std::string DATABASE_NAME = "time_data.db";

// --- 函数声明 ---
void print_full_usage(const char* app_name);
bool open_database(sqlite3** db, const std::string& db_name);
void close_database(sqlite3** db);

// --- Main function ---
int main(int argc, char* argv[]) {
    // --- 控制台设置 ---
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
    if (args.size() < 2) {
        print_full_usage(args[0].c_str());
        return 1;
    }

    const std::string command = args[1];

    // --- 优先处理通用命令 ---
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "--version" || command == "-v") {
        std::cout << "TimeMaster Command Version: " << AppInfo::VERSION << std::endl;
        std::cout << "Last Updated: " << AppInfo::LAST_UPDATED << std::endl;
        return 0;
    }

    // --- (分支1: 预处理命令 ) ---
    if (command == "-a" || command == "-c" || command == "-vs" || command == "-vo" || command == "-edc") {
        AppOptions options;
        bool path_provided = false;
        for (size_t i = 1; i < args.size(); ++i) {
            const std::string& arg = args[i];
            if (arg == "-a" || arg == "--all") options.run_all = true;
            else if (arg == "-c" || arg == "--convert") options.convert = true;
            else if (arg == "-vs" || arg == "--validate-source") options.validate_source = true;
            else if (arg == "-vo" || arg == "--validate-output") options.validate_output = true;
            else if (arg == "-edc" || arg == "--enable-day-check") options.enable_day_count_check = true;
            else if (arg.rfind("-", 0) != 0) {
                if (path_provided) { std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "提供了多个路径。" << std::endl; return 1; }
                options.input_path = arg;
                path_provided = true;
            }
        }
        if (!path_provided) { std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "预处理命令需要传入一个文件或文件夹路径。" << std::endl; return 1; }
        if (options.run_all) { options.validate_source = true; options.convert = true; options.validate_output = true; }
        AppConfig config;
        try {
            fs::path exe_path = fs::canonical(fs::path(argv[0])).parent_path();
            fs::path config_dir = exe_path / "config";
            config.interval_processor_config_path = (config_dir / "interval_processor_config.json").string();
            config.format_validator_config_path = (config_dir / "format_validator_config.json").string();
            config.error_log_path = (exe_path / "error_log.txt").string();
            if (!fs::exists(config.interval_processor_config_path) || !fs::exists(config.format_validator_config_path)) {
                throw std::runtime_error("预处理配置文件未在 config 目录中找到。");
            }
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR <<  "Fatal Error "<< RESET_COLOR << e.what() << RESET_COLOR << std::endl; return 1;
        }
        LogProcessor processor(config);
        return processor.run(options) ? 0 : 1;
    }
    // ==========================================================
    // 分支 2: 入库命令 (process/import)
    // -p, --process
    // ==========================================================
    else if (command == "-p" || command == "--process") {
        if (args.size() != 3) {
            std::cerr  << RED_COLOR << "Error: " << RESET_COLOR << command << "命令需要一个文件路径参数。\n";
            print_full_usage(args[0].c_str());
            return 1;
        }
        std::string filepath = args[2];
        std::string config_path;
        try {
            fs::path exe_path = fs::canonical(fs::path(argv[0])).parent_path();
            // 【核心修复】从 config/ 目录构建路径
            fs::path config_dir = exe_path / "config";
            config_path = (config_dir / "config.json").string();
        } catch(const std::exception& e) {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "无法确定配置文件路径: " << e.what() << std::endl;
        }

        std::cout << "Processing file '" << filepath << "' and importing data into '" << DATABASE_NAME << "'...\n";
        handle_process_files(DATABASE_NAME, filepath, config_path);
    }
    // (分支3: 查询命令 和 分支4: 未知命令 )
    else if (command == "-q" || command == "--query") {
        if (args.size() < 4) {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << " query 命令需要一个子命令和参数 (例如: -q d 20240101).\n";
            print_full_usage(args[0].c_str());
            return 1;
        }
        sqlite3* db = nullptr;
        if (!open_database(&db, DATABASE_NAME)) return 1;
        QueryHandler query_handler(db);
        std::string sub_command = args[2];
        std::string query_arg = args[3];
        if (sub_command == "d" || sub_command == "daily") {
            query_handler.run_daily_query(query_arg);
        } else if (sub_command == "p" || sub_command == "period") {
            try { query_handler.run_period_query(std::stoi(query_arg)); }
            catch (const std::exception&) { std::cerr  << RED_COLOR << "Error: " << RESET_COLOR << "<days> 参数必须是一个有效的数字。\n"; close_database(&db); return 1; }
        } else if (sub_command == "m" || sub_command == "monthly") {
            query_handler.run_monthly_query(query_arg);
        } else {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "未知的查询子命令 '" << sub_command << "'.\n";
            print_full_usage(args[0].c_str()); close_database(&db); return 1;
        }
        close_database(&db);
    }
    else {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << " 未知命令 '" << command << "'" << RESET_COLOR << std::endl;
        print_full_usage(args[0].c_str());
        return 1;
    }

    return 0;
}


void print_full_usage(const char* app_name) {
    std::cout << "TimeMaster: 一个用于时间数据预处理、入库和查询的命令行工具。\n\n";
    std::cout << "用法: " << app_name << " <命令> [参数...]\n\n";
    std::cout << GREEN_COLOR << "--- 数据预处理模块 (reprocessing) ---\n" << RESET_COLOR;
    std::cout << "  用法: " << app_name << " <标志> <文件或文件夹路径>\n";
    std::cout << "  主要动作标志:\n";
    std::cout << "    -a,  --all\t\t\t执行完整流程(检验源->转换->检验输出)。\n";
    std::cout << "    -c,  --convert\t\t仅转换文件格式。\n";
    std::cout << "    -vs, --validate-source\t仅检验源文件的格式。\n";
    std::cout << "  可选附加标志:\n";
    std::cout << "    -vo, --validate-output\t转换后检验输出文件 (需与 -c 或 -a 配合)。\n";
    std::cout << "    -edc, --enable-day-check\t启用对月份天数完整性的检查。\n";
    std::cout << "  示例: " << app_name << " -a /path/to/logs\n\n";
    std::cout << GREEN_COLOR << "--- 数据入库模块 (processing) ---\n" << RESET_COLOR;
    std::cout << "  -p, --process <filepath>\t处理单个格式化文件并导入数据库。\n";
    std::cout << "  示例: " << app_name << " -p /path/to/processed_log.txt\n\n";
    std::cout << GREEN_COLOR << "--- 数据查询模块 (queries) ---\n" << RESET_COLOR;
    std::cout << "  -q d, --query daily <YYYYMMDD>\t查询指定日期的统计信息。\n";
    std::cout << "  -q p, --query period <days>\t查询过去 N 天的统计信息。\n";
    std::cout << "  -q m, --query monthly <YYYYMM>\t查询指定月份的统计信息。\n";
    std::cout << "  示例: " << app_name << " -q m 202405\n\n";
    std::cout << GREEN_COLOR << "--- 其他选项 ---\n" << RESET_COLOR;
    std::cout << "  -h, --help\t\t\t显示此帮助信息。\n";
    std::cout << "  -v, --version\t\t\t显示程序版本。\n";
}
bool open_database(sqlite3** db, const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), db)) {
        std::cerr  << RED_COLOR << "Error: " << RESET_COLOR << " 无法打开数据库: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }
    return true;
}
void close_database(sqlite3** db) {
    if (*db) {
        sqlite3_close(*db);
        *db = nullptr;
    }
}
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sqlite3.h>
#include <filesystem>

// 包含你的项目所需的头文件
#include "common_utils.h"      
#include "processing.h"        
#include "query_handler.h"     

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// 全局常量
const std::string DATABASE_NAME = "time_data.db";

// 函数声明
void print_usage(const char* prog_name) {
    std::cout << "一个用于处理和查询时间数据的命令行工具。\n\n";
    std::cout << "用法:\n";
    std::cout << "  " << prog_name << " <command> [arguments]\n\n";
    std::cout << "可用命令:\n";
    std::cout << "  process                处理当前目录的 .txt 文件并将数据导入数据库。\n";
    std::cout << "                         (示例: " << prog_name << " process)\n\n";
    std::cout << "  query daily <YYYYMMDD> 查询指定日期的统计数据。\n";
    std::cout << "                         (示例: " << prog_name << " query daily 20240115)\n\n";
    std::cout << "  query period <days>    查询过去指定天数的统计数据。\n";
    std::cout << "                         (示例: " << prog_name << " query period 7)\n\n";
    std::cout << "  query monthly <YYYYMM> 查询指定月份的统计数据。\n";
    std::cout << "                         (示例: " << prog_name << " query monthly 202401)\n\n";
    std::cout << "  help                   显示此帮助信息。\n\n";
    // NEW: 添加 version 选项的说明
    std::cout << "其他选项:\n";
    std::cout << "  --version              显示程序版本和更新日期。\n";
}

bool open_database(sqlite3** db, const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), db)) {
        std::cerr << "数据库打开失败: " << sqlite3_errmsg(*db) << std::endl;
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
void setup_console() {
    #if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    #endif
}

// --- 主函数 ---
int main(int argc, char* argv[]) {
    setup_console();

    // 将C风格的参数转换为更易于管理的 std::vector<std::string>
    std::vector<std::string> args(argv, argv + argc);
    const std::string version = "1.0.0";
    const std::string updated = "2025-06-24" ;

    // 1. 基本的参数数量检查
    if (args.size() < 2) {
        print_usage(args[0].c_str());
        return 1;
    }

    // 在所有其他命令之前优先检查 --version 标志
    if (args[1] == "--version") {
        std::cout << "time_tracker_command Version: " << version << std::endl;
        std::cout << "Last Updated:"<<  updated << std::endl;
        return 0; // 打印后立即成功退出
    }

    // 2. 获取主命令
    std::string command = args[1];

    // 3. 根据主命令执行不同逻辑
    if (command == "process") {
        // ... (这部分代码保持不变)
        if (args.size() != 2) { // process 命令现在不接受额外路径参数
            std::cerr << "错误: 'process' 命令不接受额外参数。\n";
            print_usage(args[0].c_str());
            return 1;
        }
        std::cout << "注意: 'process' 命令会处理当前目录下的 txt 文件并存入 '" << DATABASE_NAME << "'\n";
        handle_process_files(DATABASE_NAME); 

    } else if (command == "query") {
        if (args.size() < 3) {
            std::cerr << "错误: 'query' 命令需要一个子命令 (例如 daily, period, monthly)。\n";
            print_usage(args[0].c_str());
            return 1;
        }

        sqlite3* db = nullptr;
        if (!open_database(&db, DATABASE_NAME)) {
            return 1; // 打开数据库失败
        }

        QueryHandler query_handler(db);
        std::string sub_command = args[2];

        if (sub_command == "daily") {
            if (args.size() != 4) {
                std::cerr << "错误: 'query daily' 需要一个日期参数 (YYYYMMDD)。\n";
                close_database(&db);
                return 1;
            }
            query_handler.run_daily_query(args[3]);

        } else if (sub_command == "period") {
            if (args.size() != 4) {
                std::cerr << "错误: 'query period' 需要一个天数参数 (例如 7, 14, 30)。\n";
                close_database(&db);
                return 1;
            }
            try {
                int days = std::stoi(args[3]);
                query_handler.run_period_query(days);
            } catch (const std::exception& e) {
                std::cerr << "错误: 天数必须是一个有效的数字。\n";
                close_database(&db);
                return 1;
            }
        } else if (sub_command == "monthly") {
            if (args.size() != 4) {
                std::cerr << "错误: 'query monthly' 需要一个月份参数 (YYYYMM)。\n";
                close_database(&db);
                return 1;
            }
            query_handler.run_monthly_query(args[3]);
        } else {
            std::cerr << "错误: 未知的 query 子命令 '" << sub_command << "'。\n";
            print_usage(args[0].c_str());
            close_database(&db);
            return 1;
        }

        close_database(&db);

    } else if (command == "help" || command == "--help") {
        print_usage(args[0].c_str());
    }
    else {
        std::cerr << "错误: 未知的主命令 '" << command << "'。\n";
        print_usage(args[0].c_str());
        return 1;
    }

    return 0;
}









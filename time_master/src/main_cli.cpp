
#include <iostream> // 为使用 std::cerr 添加
#include <print>
#include <string>
#include <vector>
#include <stdexcept>


// --- Windows-specific include for console functions ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "common/common_utils.hpp"
#include "common/version.hpp"
#include "time_master_cli/CliController.hpp"

// --- Function Declarations ---
void print_full_usage(const char* app_name);

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
    if (args.size() < 2) {
        print_full_usage(args[0].c_str());
        return 1;
    }

    // [修改 1] 识别并处理缩写命令
    // 在将参数传递给控制器之前，将 "pre" 替换为 "preprocess"
    if (args[1] == "pre") {
        args[1] = "preprocess";
    }

    const std::string& command = args[1];

    // --- Handle simple, top-level commands directly ---
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "-v" || command == "--version") {
        std::println("TimeMaster Command Version: {}", AppInfo::VERSION);
        std::println("Last Updated:  {}", AppInfo::LAST_UPDATED);
        return 0;
    }

    // --- Delegate complex commands to the controller ---
    try {
        // 控制器接收的是已经“翻译”好的完整命令
        CliController controller(args);
        controller.execute();
    } catch (const std::exception& e) {
        // 使用 std::println 将格式化的错误信息输出到 std::cerr
        std::println(std::cerr, "{}{}{}{}", RED_COLOR, "Error: ", RESET_COLOR, e.what());
        
        // Optionally show usage for command-related errors
        if (std::string(e.what()).find("command") != std::string::npos || 
            std::string(e.what()).find("argument") != std::string::npos) {
             std::println("\nUse '{}' for more information.", args[0]);
        }
        return 1;
    }

    return 0;
}

void print_full_usage(const char* app_name) {
    std::println("{}TimeMaster{}: A command-line tool for time data pre-processing, import, and querying.\n",GREEN_COLOR,RESET_COLOR);
    std::println("Usage: {} <command>[arguments...] [options...]\n", app_name);
    
 std::println("{}{}{}", GREEN_COLOR, "--- Core Commands ---", RESET_COLOR);
    std::println("  run-pipeline <path>\t\t Run full pipeline: validate source, convert, and validate output.");
    std::println("  import <path>\t\t\t Import processed .json files into the database.");
    std::println("  query <type> <period>\t\t Query data from the database.");
    std::println("  export <type> <period>\t Export reports from the database.\n");

    std::println("{}{}{}", GREEN_COLOR, "--- Pre-processing Commands ---", RESET_COLOR);
    std::println("  validate-source <path>\t Validates the source file format (e.g., .txt files).");
    std::println("  convert <path>\t\t Converts source files to the processed JSON format.");
    std::println("  validate-output <path>\t Validates the processed JSON file format and logic.");
    std::println("    Options for validate-output:");
    std::println("      --enable-day-check, -edc\t Enable check for day completeness in a month.");
    std::println("  Example: {} convert /path/to/logs", app_name);
    std::println("  Example: {} validate-output /path/to/processed/log.json --enable-day-check\n", app_name);

    std::println("{}{}{}", GREEN_COLOR, "--- Command: import ---", RESET_COLOR);
    std::println("  Usage: {} import <directory_path>", app_name);
    std::println("  Example: {} import /path/to/processed_logs/\n", app_name);
    
    std::println("{}{}{}", GREEN_COLOR, "--- Command: query ---", RESET_COLOR);
    std::println("  Usage: {} query <type> <argument> [options...]", app_name);
    std::println("  Types:");
    std::println("    daily <YYYYMMDD>\t\t Query statistics for a specific day.");
    std::println("    monthly <YYYYMM>\t\t Query statistics for a specific month.");
    std::println("    period <days>\t\t Query statistics for last N days. Can be a list (e.g., 7,30).");
    std::println("  Options (for all query types):");
    std::println("    --format, -f <format>\t Specify output format (md, tex, typ). Default: md.");
    std::println("  Example: {} query daily 20240101 --format tex\n", app_name);
    
    std::println("{}{}{}", GREEN_COLOR, "--- Command: export ---", RESET_COLOR);
    std::println("  Usage: {} export <type> [argument] [options...]", app_name);
    std::println("  Types:");
    std::println("    daily <YYYYMMDD>\t\t Export a single daily report.");
    std::println("    monthly <YYYYMM>\t\t Export a single monthly report.");
    std::println("    period <days>\t\t Export a single period report (e.g., 7).");
    std::println("    all-daily\t\t\t Export all daily reports.");
    std::println("    all-monthly\t\t\t Export all monthly reports.");
    std::println("    all-period <days_list>\t Export multiple period reports (e.g., 7,30,90).");
    std::println("  Options (for all export types):");
    std::println("    --format, -f <format>\t Specify output format (md, tex, typ). Default: md.");
    std::println("  Example: {} export daily 20240115 --format tex", app_name);
    std::println("  Example: {} export all-monthly --format tex\n", app_name);

    std::println("{}{}{}", GREEN_COLOR, "--- Other Options ---", RESET_COLOR);
    std::println("  --help, -h\t\t\t Show this help message.");
    std::println("  --version, -v\t\t\t Show program version.");
}

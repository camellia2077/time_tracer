#include "common/pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <print>

// --- Windows-specific include for console functions ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "common/common_utils.h"
#include "common/version.h"
#include "time_master_cli/CliController.h" // [新增] 包含新的控制器

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

    const std::string& command = args[1];

    // --- Handle simple commands directly ---
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "--version" || command == "-v") {
        std::println("TimeMaster Command Version: {}", AppInfo::VERSION);
        std::println("Last Updated:  {}", AppInfo::LAST_UPDATED);
        return 0;
    }

    // --- Delegate complex commands to the controller ---
    try {
        CliController controller(args);
        controller.execute();
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << e.what() << std::endl;
        // Optionally show usage for command-related errors
        if (std::string(e.what()).find("command") != std::string::npos || 
            std::string(e.what()).find("requires") != std::string::npos) {
             print_full_usage(args[0].c_str());
        }
        return 1;
    }

    return 0;
}

void print_full_usage(const char* app_name) {
    std::cout << "TimeMaster: A command-line tool for time data pre-processing, import, and querying.\n\n";
    std::cout << "Usage: " << app_name << " <command> [arguments...]\n\n";
    
    std::cout << GREEN_COLOR << "--- Full Pipeline ---\n" << RESET_COLOR;
    std::cout << "  --all, -a <path>\t\tExecute full flow: validate source, convert, and import into database.\n";
    std::cout << "  Example: " << app_name << " --all /path/to/source_logs\n\n";
    
    std::cout << GREEN_COLOR << "--- Manual Pre-processing Steps ---\n" << RESET_COLOR;
    std::cout << "  Usage: " << app_name << " <flag(s)> <file_or_folder_path>\n";
    std::cout << "  Action Flags:\n";
    std::cout << "    --validate-source, -vs\tOnly validate the source file format.\n";
    std::cout << "    --convert, -c\t\tOnly convert file format.\n";
    std::cout << "  Optional Flags (used with action flags):\n";
    std::cout << "    --validate-output, -vo\tValidate output file after conversion (use with -c).\n";
    std::cout << "    --enable-day-check, -edc\tEnable check for completeness of days in a month (use with -vo).\n";
    std::cout << "  Example: " << app_name << " --validate-source --convert --validate-output /path/to/logs\n\n";
    
    std::cout << GREEN_COLOR << "--- Manual Data Import ---\n" << RESET_COLOR;
    std::cout << "  --process, -p <path>\t\tProcess a directory of formatted .txt files and import to database.\n";
    std::cout << "  Example: " << app_name << " --process /path/to/processed_logs/\n\n";
    
    std::cout << GREEN_COLOR << "--- Data Query Module ---\n" << RESET_COLOR;
    std::cout << "  --query daily, -q d <YYYYMMDD>\tQuery statistics for a specific day.\n";
    std::cout << "  --query monthly, -q m <YYYYMM>\tQuery statistics for a specific month.\n";
    std::cout << "  --query period, -q p <days>\t\tQuery statistics for last N days. Can be a list (e.g., 7,30).\n";
    std::cout << "  Optional (for ALL queries):\n";
    std::cout << "    --format, -f <format>\t\tSpecify output format (e.g., md, tex, typ). Default is md.\n";
    std::cout << "  Example: " << app_name << " --query daily 20240101 --format tex\n\n";
    
    std::cout << GREEN_COLOR << "--- Data Export Module ---\n" << RESET_COLOR;
    std::cout << "  --export daily <YYYYMMDD>\t\tExport a single daily report.\n";
    std::cout << "  --export monthly <YYYYMM>\t\tExport a single monthly report.\n";
    std::cout << "  --export period <days>\t\tExport period reports for given days (e.g., 7,30,90).\n";
    std::cout << "  --export all-daily\t\t\tExport all daily reports.\n";
    std::cout << "  --export all-monthly\t\t\tExport all monthly reports.\n";
    std::cout << "  Optional (for ALL exports):\n";
    std::cout << "    --format, -f <format>\t\tSpecify output format (e.g., md, tex, typ). Default is md.\n";
    std::cout << "  Example: " << app_name << " --export daily 20240115 --format tex\n";
    std::cout << "  Example: " << app_name << " --export all-daily --format tex\n\n";

    std::cout << GREEN_COLOR << "--- Other Options ---\n" << RESET_COLOR;
    std::cout << "  --help, -h\t\t\tShow this help message.\n";
    std::cout << "  --version, -v\t\t\tShow program version.\n";
}
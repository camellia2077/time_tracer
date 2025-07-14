#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <print>

// --- Windows-specific include for console functions ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "common_utils.h"
#include "version.h"

#include "FileController.h" 
#include "ActionHandler.h"
#include "QueryHandler.h"
#include "LogProcessor.h"

// --- Global Constants ---
const std::string DATABASE_NAME = "time_data.db";

// --- Function Declarations ---
void print_full_usage(const char* app_name);
bool open_database(sqlite3** db, const std::string& db_name);
void close_database(sqlite3** db);


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

    const std::string command = args[1];

    // --- General Commands ---
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "--version" || command == "-v") {
        std::println("TimeMaster Command Version: {}",AppInfo::VERSION);
        std::println("Last Updated:  {}",AppInfo::LAST_UPDATED);
        return 0;
    }

    // --- 主逻辑块,包含初始化和命令分派 ---
    try {
        // --- Unified Initialization using FileController ---
        FileController file_controller(argv[0]);

        ActionHandler action_handler(
            DATABASE_NAME,
            file_controller.get_config(),
            file_controller.get_main_config_path()
        );
        
        // Branch 1: Full Pipeline (-a, --all)
        if (command == "-a" || command == "--all") {
            if (args.size() != 3) {
                throw std::runtime_error("Command '" + command + "' requires exactly one source directory path.");
            }
            action_handler.run_full_pipeline_and_import(args[2]);
        }
        // Branch 2: Manual Pre-processing Steps (e.g., -c, -vs)
        else if (command == "-c" || command == "--convert" || command == "-vs" || command == "--validate-source" || command == "-vo" || command == "--validate-output" || command == "-edc" || command == "--enable-day-check") {
            bool convert_flag = false;
            bool validate_source_flag = false;
            bool validate_output_flag = false;
            bool day_check_flag = false;
            std::string input_path;
            bool path_provided = false;

            for (size_t i = 1; i < args.size(); ++i) {
                const std::string& arg = args[i];
                if (arg == "-c" || arg == "--convert") convert_flag = true;
                else if (arg == "-vs" || arg == "--validate-source") validate_source_flag = true;
                else if (arg == "-vo" || arg == "--validate-output") validate_output_flag = true;
                else if (arg == "-edc" || arg == "--enable-day-check") day_check_flag = true;
                else if (arg.rfind("-", 0) != 0) { // Argument is a path
                    if (path_provided) throw std::runtime_error("Multiple paths provided for file processing.");
                    input_path = arg;
                    path_provided = true;
                }
            }

            if (!path_provided) {
                throw std::runtime_error("A file or folder path is required for manual pre-processing commands.");
            }
            if (validate_output_flag && !convert_flag) {
                throw std::runtime_error("The --validate-output (-vo) flag can only be used with the --convert (-c) flag.");
            }

            if (!action_handler.collectFiles(input_path)) {
                 std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Failed to collect files from the specified path. Aborting." << std::endl;
                 return 1;
            }
            if (validate_source_flag) {
                if (!action_handler.validateSourceFiles()) return 1;
            }
            if (convert_flag) {
                if (!action_handler.convertFiles()) return 1;
            }
            if (validate_output_flag) {
                if (!action_handler.validateOutputFiles(day_check_flag)) return 1;
            }
        }
        // Branch 3: Database import (-p, --process)
        else if (command == "-p" || command == "--process") {
            if (args.size() != 3) throw std::runtime_error("Command '" + command + "' requires exactly one directory path.");
            
            std::cout << YELLOW_COLOR << "Warning:\n" << RESET_COLOR;
            std::cout << "This -p command is designed to import log files that have already been pre-processed (-vs -c -vo).\n";
            std::cout << "Importing raw or unvalidated log files will lead to incorrect data in the database and cause query (-q) functions to fail.\n";
            std::cout << "Please ensure the specified path contains only converted and pre-processed files.\n\n";
            std::cout << "Are you sure you want to continue? (y/n): ";

            char confirmation;
            std::cin >> confirmation;

            if (confirmation != 'y' && confirmation != 'Y') {
                std::cout << RED_COLOR << "\nOperation cancelled." << RESET_COLOR << std::endl;
                return 0;
            }
            
            std::cout << std::endl;
            action_handler.run_database_import(args[2]);
        }
        // Branch 4: Query (-q)
        else if (command == "-q" || command == "--query") {
            if (args.size() < 4) {
                std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Query command requires a sub-command and an argument (e.g., -q d 20240101).\n";
                print_full_usage(args[0].c_str());
                return 1;
            }
        
            std::string sub_command = args[2];
            std::string query_arg = args[3];
        
            if (sub_command == "d" || sub_command == "daily") {
                std::cout << action_handler.run_daily_query(query_arg);
            } else if (sub_command == "p" || sub_command == "period") {
                try {
                    std::cout << action_handler.run_period_query(std::stoi(query_arg));
                } catch (const std::exception&) {
                    std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "<days> argument must be a valid number.\n";
                    return 1;
                }
            } else if (sub_command == "m" || sub_command == "monthly") {
                std::cout << action_handler.run_monthly_query(query_arg);
            } else {
                std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Unknown query sub-command '" << sub_command << "'.\n";
                print_full_usage(args[0].c_str());
                return 1;
            }
        }
        // Branch 5: Data Export (-export, -e)
        else if (command == "-export" || command == "-e") {
            if (args.size() != 3) {
                throw std::runtime_error("Command '" + command + "' requires a sub-command (e.g., -export day).");
            }
            std::string sub_command = args[2];
            if (sub_command == "day" || sub_command == "d") {
                action_handler.run_export_all_daily_reports_query();
            } else if (sub_command == "month") { // [新增] month 子命令
                action_handler.run_export_all_monthly_reports_query();
            }
            else {
                std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Unknown export sub-command '" << sub_command << "'.\n";
                print_full_usage(args[0].c_str());
                return 1;
            }
        }
        // Unknown command
        else {
            throw std::runtime_error("Unknown command '" + command + "'.");
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << e.what() << std::endl;
        if (std::string(e.what()).find("command") != std::string::npos) {
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
    std::cout << "  -a,  --all <path>\t\tExecute full flow: validate source, convert, and import into database.\n";
    std::cout << "  Example: " << app_name << " -a /path/to/source_logs\n\n";
    std::cout << GREEN_COLOR << "--- Manual Pre-processing Steps ---\n" << RESET_COLOR;
    std::cout << "  Usage: " << app_name << " <flag(s)> <file_or_folder_path>\n";
    std::cout << "  Action Flags:\n";
    std::cout << "    -vs, --validate-source\tOnly validate the source file format.\n";
    std::cout << "    -c,  --convert\t\tOnly convert file format.\n";
    std::cout << "  Optional Flags (used with action flags):\n";
    std::cout << "    -vo, --validate-output\tValidate output file after conversion (use with -c).\n";
    std::cout << "    -edc, --enable-day-check\tEnable check for completeness of days in a month (use with -vo).\n";
    std::cout << "  Example: " << app_name << " -vs -c -vo /path/to/logs\n\n";
    std::cout << GREEN_COLOR << "--- Manual Data Import ---\n" << RESET_COLOR;
    std::cout << "  -p, --process <path>\t\tProcess a directory of formatted .txt files and import to database.\n";
    std::cout << "  Example: " << app_name << " -p /path/to/processed_logs/\n\n";
    std::cout << GREEN_COLOR << "--- Data Query Module ---\n" << RESET_COLOR;
    std::cout << "  -q d, --query daily <YYYYMMDD>\tQuery statistics for a specific day.\n";
    std::cout << "  -q p, --query period <days>\t\tQuery statistics for the last N days.\n";
    std::cout << "  -q m, --query monthly <YYYYMM>\tQuery statistics for a specific month.\n";
    std::cout << "  Example: " << app_name << " -q m 202405\n\n";
    // [修改] 更新导出指令的帮助文档
    std::cout << GREEN_COLOR << "--- Data Export Module ---\n" << RESET_COLOR;
    std::cout << "  -export day (-e d)\t\tExport all daily reports to individual .md files.\n";
    std::cout << "  -export month\t\t\tExport all monthly reports to combined .md files.\n";
    std::cout << "  Example: " << app_name << " -export month\n\n";
    std::cout << GREEN_COLOR << "--- Other Options ---\n" << RESET_COLOR;
    std::cout << "  -h, --help\t\t\tShow this help message.\n";
    std::cout << "  -v, --version\t\t\tShow program version.\n";
}

bool open_database(sqlite3** db, const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), db)) {
        std::cerr  << RED_COLOR << "Error: " << RESET_COLOR << " Cannot open database: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }
    return true;
}

void close_database(sqlite3** db) {
    if (db && *db) {
        sqlite3_close(*db);
        *db = nullptr;
    }
}

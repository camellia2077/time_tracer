#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// --- Windows-specific include for console functions ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "common_utils.h"
#include "version.h"
#include "FileHandler.h"
#include "ActionHandler.h"

// --- Global Constants ---
const std::string DATABASE_NAME = "time_data.db";

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

    const std::string command = args[1];

    // --- General Commands ---
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "--version" || command == "-v") {
        std::cout << "TimeMaster Command Version: " << AppInfo::VERSION << std::endl;
        std::cout << "Last Updated: " << AppInfo::LAST_UPDATED << std::endl;
        return 0;
    }

    // --- Unified Initialization ---
    AppConfig config;
    std::string main_config_path_str;
    try {
        FileHandler file_handler(argv[0]);
        config = file_handler.load_configuration();
        main_config_path_str = file_handler.get_main_config_path();
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Fatal Error: " << RESET_COLOR << e.what() << std::endl;
        return 1;
    }

    // Create the core controller
    ActionHandler action_handler(DATABASE_NAME, config, main_config_path_str);

    // --- Delegate all branches to ActionHandler ---
    try {
        // Branch 1: Full Pipeline (-a, --all)
        if (command == "-a" || command == "--all") {
            if (args.size() != 3) {
                throw std::runtime_error("Command '" + command + "' requires exactly one source directory path.");
            }
            action_handler.run_full_pipeline_and_import(args[2]);
        }
        // Branch 2: File pre-processing (-c, -vs, etc.)
        else if (command == "-c" || command == "-vs" || command == "-vo" || command == "-edc") {
            AppOptions options;
            bool path_provided = false;
            // Parse all file-processing related arguments
            for (size_t i = 1; i < args.size(); ++i) {
                const std::string& arg = args[i];
                if (arg == "-c" || arg == "--convert") options.convert = true;
                else if (arg == "-vs" || arg == "--validate-source") options.validate_source = true;
                else if (arg == "-vo" || arg == "--validate-output") options.validate_output = true;
                else if (arg == "-edc" || arg == "--enable-day-check") options.enable_day_count_check = true;
                else if (arg.rfind("-", 0) != 0) { // If the argument doesn't start with '-', it's a path
                    if (path_provided) throw std::runtime_error("Multiple paths provided for file processing.");
                    options.input_path = arg;
                    path_provided = true;
                }
            }
            if (!path_provided) throw std::runtime_error("A file or folder path is required for these commands.");
            
            action_handler.run_log_processing(options);
        }
        // Branch 3: Database import (-p, --process)
        else if (command == "-p" || command == "--process") {
            if (args.size() != 3) throw std::runtime_error("Command '" + command + "' requires exactly one directory path.");
            action_handler.run_database_import(args[2]);
        }
        // Branch 4: Query (-q)
        else if (command == "-q" || command == "--query") {
            if (args.size() < 4) throw std::runtime_error("Query command requires a sub-command and an argument.");
            const std::string sub_command = args[2];
            const std::string query_arg = args[3];
            if (sub_command == "d" || sub_command == "daily") action_handler.run_daily_query(query_arg);
            else if (sub_command == "p" || sub_command == "period") action_handler.run_period_query(std::stoi(query_arg));
            else if (sub_command == "m" || sub_command == "monthly") action_handler.run_monthly_query(query_arg);
            else throw std::runtime_error("Unknown query sub-command '" + sub_command + "'.");
        }
        // Unknown command
        else {
            throw std::runtime_error("Unknown command '" + command + "'.");
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << e.what() << std::endl;
        print_full_usage(args[0].c_str());
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
    std::cout << "    -c,  --convert\t\tOnly convert file format.\n";
    std::cout << "    -vs, --validate-source\tOnly validate the source file format.\n";
    std::cout << "  Optional Flags:\n";
    std::cout << "    -vo, --validate-output\tValidate output file after conversion (use with -c).\n";
    std::cout << "    -edc, --enable-day-check\tEnable check for completeness of days in a month.\n";
    std::cout << "  Example: " << app_name << " -c -vo /path/to/logs\n\n";
    std::cout << GREEN_COLOR << "--- Manual Data Import ---\n" << RESET_COLOR;
    std::cout << "  -p, --process <path>\t\tProcess a directory of formatted .txt files and import to database.\n";
    std::cout << "  Example: " << app_name << " -p /path/to/processed_logs/\n\n";
    std::cout << GREEN_COLOR << "--- Data Query Module ---\n" << RESET_COLOR;
    std::cout << "  -q d, --query daily <YYYYMMDD>\tQuery statistics for a specific day.\n";
    std::cout << "  -q p, --query period <days>\t\tQuery statistics for the last N days.\n";
    std::cout << "  -q m, --query monthly <YYYYMM>\tQuery statistics for a specific month.\n";
    std::cout << "  Example: " << app_name << " -q m 202405\n\n";
    std::cout << GREEN_COLOR << "--- Other Options ---\n" << RESET_COLOR;
    std::cout << "  -h, --help\t\t\tShow this help message.\n";
    std::cout << "  -v, --version\t\t\tShow program version.\n";
}
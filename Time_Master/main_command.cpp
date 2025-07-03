#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include <sqlite3.h>

#include "FileHandler.h"

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
#include "FileHandler.h" // <<< REFACTOR: Include the FileHandler module
#include <nlohmann/json.hpp>


// --- Namespace alias ---
namespace fs = std::filesystem;

// --- Global Constants ---
const std::string DATABASE_NAME = "time_data.db";

// --- Function Declarations ---
void print_full_usage(const char* app_name);
bool open_database(sqlite3** db, const std::string& db_name);
void close_database(sqlite3** db);

// --- Main function ---
int main(int argc, char* argv[]) {
    // --- Console Setup ---
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

    // --- Handle general commands first ---
    if (command == "-h" || command == "--help") {
        print_full_usage(args[0].c_str());
        return 0;
    }
    if (command == "--version" || command == "-v") {
        std::cout << "TimeMaster Command Version: " << AppInfo::VERSION << std::endl;
        std::cout << "Last Updated: " << AppInfo::LAST_UPDATED << std::endl;
        return 0;
    }

    // --- Branch 1: Pre-processing commands (-a, -c, etc.) ---
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
                if (path_provided) { std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Multiple paths provided." << std::endl; return 1; }
                options.input_path = arg;
                path_provided = true;
            }
        }
        if (!path_provided) { std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "A file or folder path is required for pre-processing commands." << std::endl; return 1; }
        if (options.run_all) { options.validate_source = true; options.convert = true; options.validate_output = true; }
        
        AppConfig config;
        try {
            // <<< REFACTOR: Use FileHandler to load configuration.
            FileHandler file_handler(argv[0]);
            config = file_handler.load_configuration();
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR <<  "Fatal Error: "<< RESET_COLOR << e.what() << std::endl; 
            return 1;
        }
        
        LogProcessor processor(config);
        return processor.run(options) ? 0 : 1;
    }
    // --- Branch 2: Database import command (-p, --process) ---
    else if (command == "-p" || command == "--process") {
        if (args.size() != 3) {
            std::cerr  << RED_COLOR << "Error: " << RESET_COLOR << command << " command requires a single file or directory path.\n";
            print_full_usage(args[0].c_str());
            return 1;
        }

        std::string config_path;
        try {
            // <<< REFACTOR: Use FileHandler to safely get the config path.
            FileHandler file_handler(argv[0]);
            config_path = file_handler.get_main_config_path();
        } catch(const std::exception& e) {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Could not determine configuration file path: " << e.what() << std::endl;
            return 1;
        }

        fs::path input_path = args[2];

        if (!fs::exists(input_path)) {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "The specified path does not exist: " << input_path.string() << std::endl;
            return 1;
        }

        // Handle both directory and single file inputs ---
        if (fs::is_directory(input_path)) {
            std::cout << "Processing all .txt files in directory '" << input_path.string() << "'...\n";
            
            // 使用新方法递归查找文件
            std::vector<fs::path> files_to_process = FileHandler::find_text_files_recursively(input_path);

            if (files_to_process.empty()) {
                std::cout << YELLOW_COLOR << "Warning: " << RESET_COLOR << "No .txt files found in the specified directory.\n";
            } else {
                // 遍历找到的文件列表
                for (const auto& file_path : files_to_process) {
                    std::cout << "\n--- Processing file: " << file_path.filename().string() << " ---\n";
                    handle_process_files(DATABASE_NAME, file_path.string(), config_path);
                }
                std::cout << "\nFinished processing " << files_to_process.size() << " file(s).\n";
            }
        } else { // It's a single file
            std::cout << "Processing file '" << input_path.string() << "' and importing data into '" << DATABASE_NAME << "'...\n";
            handle_process_files(DATABASE_NAME, input_path.string(), config_path);
        }
    }
    // --- Branch 3: Query commands ---
    else if (command == "-q" || command == "--query") {
        if (args.size() < 4) {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << " query command requires a sub-command and argument (e.g., -q d 20240101).\n";
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
            catch (const std::exception&) { std::cerr  << RED_COLOR << "Error: " << RESET_COLOR << "<days> argument must be a valid number.\n"; close_database(&db); return 1; }
        } else if (sub_command == "m" || sub_command == "monthly") {
            query_handler.run_monthly_query(query_arg);
        } else {
            std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Unknown query sub-command '" << sub_command << "'.\n";
            print_full_usage(args[0].c_str()); close_database(&db); return 1;
        }
        close_database(&db);
    }
    // --- Branch 4: Unknown command ---
    else {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << " Unknown command '" << command << "'" << RESET_COLOR << std::endl;
        print_full_usage(args[0].c_str());
        return 1;
    }

    return 0;
}


void print_full_usage(const char* app_name) {
    std::cout << "TimeMaster: A command-line tool for time data pre-processing, import, and querying.\n\n";
    std::cout << "Usage: " << app_name << " <command> [arguments...]\n\n";
    std::cout << GREEN_COLOR << "--- Data Pre-processing Module (reprocessing) ---\n" << RESET_COLOR;
    std::cout << "  Usage: " << app_name << " <flag> <file_or_folder_path>\n";
    std::cout << "  Main Action Flags:\n";
    std::cout << "    -a,  --all\t\t\tExecute full flow (validate source -> convert -> validate output).\n";
    std::cout << "    -c,  --convert\t\tOnly convert file format.\n";
    std::cout << "    -vs, --validate-source\tOnly validate the source file format.\n";
    std::cout << "  Optional Flags:\n";
    std::cout << "    -vo, --validate-output\tValidate output file after conversion (use with -c or -a).\n";
    std::cout << "    -edc, --enable-day-check\tEnable check for completeness of days in a month.\n";
    std::cout << "  Example: " << app_name << " -a /path/to/logs\n\n";
    std::cout << GREEN_COLOR << "--- Data Import Module (processing) ---\n" << RESET_COLOR;
    std::cout << "  -p, --process <path>\t\tProcess a single formatted .txt file or a directory of .txt files and import to database.\n";
    std::cout << "  Example (file): " << app_name << " -p /path/to/processed_log.txt\n";
    std::cout << "  Example (dir):  " << app_name << " -p /path/to/processed_logs/\n\n";
    std::cout << GREEN_COLOR << "--- Data Query Module (queries) ---\n" << RESET_COLOR;
    std::cout << "  -q d, --query daily <YYYYMMDD>\tQuery statistics for a specific day.\n";
    std::cout << "  -q p, --query period <days>\t\tQuery statistics for the last N days.\n";
    std::cout << "  -q m, --query monthly <YYYYMM>\tQuery statistics for a specific month.\n";
    std::cout << "  Example: " << app_name << " -q m 202405\n\n";
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
    if (*db) {
        sqlite3_close(*db);
        *db = nullptr;
    }
}
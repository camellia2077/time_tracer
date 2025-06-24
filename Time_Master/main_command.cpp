#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sqlite3.h>
#include <filesystem>

// Include necessary header files for your project
#include "common_utils.h"      
#include "processing.h"        
#include "query_handler.h"     

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// Global constants
const std::string DATABASE_NAME = "time_data.db";

// Function to print usage instructions
void print_usage(const char* prog_name) {
    std::cout << "A command-line tool for processing and querying time data.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << prog_name << " <command> [arguments]\n\n";
    std::cout << "Available commands:\n";
    std::cout << "  -p, --process <filepath>        Process the file at the specified path and import its data.\n";
    std::cout << "                                  (Example: " << prog_name << " -p /path/to/data.txt)\n\n";
    std::cout << "  -q d, --query daily <YYYYMMDD>  Query statistics for a specific date.\n";
    std::cout << "                                  (Example: " << prog_name << " -q d 20250528)\n\n";
    std::cout << "  -q p, --query period <days>     Query statistics for the last specified number of days.\n";
    std::cout << "                                  (Example: " << prog_name << " -q p 30)\n\n";
    std::cout << "  -q m, --query monthly <YYYYMM>  Query statistics for a specific month.\n";
    std::cout << "                                  (Example: " << prog_name << " -q m 202506)\n\n";
    std::cout << "  -h, --help                      Show this help message.\n\n";
    std::cout << "Other options:\n";
    std::cout << "  --version                       Show program version and update date.\n";
}

// Function to open the SQLite database
bool open_database(sqlite3** db, const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), db)) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }
    return true;
}

// Function to close the SQLite database
void close_database(sqlite3** db) {
    if (*db) {
        sqlite3_close(*db);
        *db = nullptr;
    }
}

// Function to set up console for UTF-8 and virtual terminal processing on Windows
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

// --- Main function ---
int main(int argc, char* argv[]) {
    setup_console();

    // Convert C-style arguments to a more manageable std::vector<std::string>
    std::vector<std::string> args(argv, argv + argc);
    const std::string version = "1.2.0";
    const std::string updated = "2025-06-24";

    if (args.size() < 2) {
        print_usage(args[0].c_str());
        return 1;
    }

    // Handle --version and --help/ -h flags first
    if (args[1] == "--version") {
        std::cout << "time_tracker_command Version: " << version << std::endl;
        std::cout << "Last Updated: " << updated << std::endl;
        return 0;
    }

    if (args[1] == "-h" || args[1] == "--help") {
        print_usage(args[0].c_str());
        return 0;
    }

    std::string command = args[1];

    if (command == "-p" || command == "--process") {
        if (args.size() != 3) {
            std::cerr << "Error: '" << command << "' command requires a <filepath> argument.\n";
            print_usage(args[0].c_str());
            return 1;
        }
        std::string filepath = args[2];
        std::cout << "Processing file '" << filepath << "' and importing data into '" << DATABASE_NAME << "'...\n";
        handle_process_files(DATABASE_NAME, filepath); 

    } else if (command == "-q" || command == "--query") {
        if (args.size() < 4) {
            std::cerr << "Error: 'query' command requires a subcommand and an argument (e.g., d <YYYYMMDD>).\n";
            print_usage(args[0].c_str());
            return 1;
        }

        sqlite3* db = nullptr;
        if (!open_database(&db, DATABASE_NAME)) {
            return 1; // Failed to open database
        }

        QueryHandler query_handler(db);
        std::string sub_command = args[2];
        std::string query_arg = args[3];

        if (sub_command == "d" || sub_command == "daily") {
            query_handler.run_daily_query(query_arg);
        } else if (sub_command == "p" || sub_command == "period") {
            try {
                int days = std::stoi(query_arg);
                query_handler.run_period_query(days);
            } catch (const std::exception& e) {
                std::cerr << "Error: The <days> argument must be a valid number.\n";
                close_database(&db);
                return 1;
            }
        } else if (sub_command == "m" || sub_command == "monthly") {
            query_handler.run_monthly_query(query_arg);
        } else {
            std::cerr << "Error: Unknown query subcommand '" << sub_command << "'.\n";
            print_usage(args[0].c_str());
            close_database(&db);
            return 1;
        }

        close_database(&db);

    } else {
        std::cerr << "Error: Unknown command '" << command << "'.\n";
        print_usage(args[0].c_str());
        return 1;
    }

    return 0;
}
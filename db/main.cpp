#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <algorithm>
#include <iomanip> // For std::setprecision and std::fixed
#include <sqlite3.h>
#include <filesystem>
#include <chrono>

#include "common_utils.h"
#include "data_parser.h"
#include "database_importer.h"
#include "query_handler.h" // Main query handler header

// Define a namespace alias for convenience
namespace fs = std::filesystem;

// Declare ANSI escape codes for text colors
const std::string ANSI_COLOR_GREEN = "\x1b[32m";
const std::string ANSI_COLOR_RESET = "\x1b[0m";

const std::string DATABASE_NAME = "time_data.db";

// Forward declarations
void handle_process_files();

// =================================================================
// Refactored Class: FileFinder
// =================================================================
/**
 * @class FileFinder
 * @brief Handles discovery of files to be processed.
 *
 * It prompts the user for input paths (files or directories),
 * validates them, and recursively finds all .txt files.
 */
class FileFinder {
public:
    FileFinder() = default;

    /**
     * @brief The main public method to collect user input and find files.
     * @return A sorted vector of strings, where each string is a path to a .txt file.
     */
    std::vector<std::string> collect_and_find_files() {
        std::vector<std::string> user_inputs = get_user_inputs();
        if (user_inputs.empty()) {
            std::cout << "No filenames or directories entered." << std::endl;
            return {};
        }

        std::vector<std::string> found_files = find_txt_files_in_paths(user_inputs);
        std::sort(found_files.begin(), found_files.end());
        return found_files;
    }

private:
    /**
     * @brief Prompts the user and gets a list of input paths.
     * @return A vector of strings representing the user's input.
     */
    std::vector<std::string> get_user_inputs() {
        std::cout << "Enter file name(s) or directory path(s) to process (space-separated, then Enter): ";
        std::string line;
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> user_inputs;
        while (ss >> token) {
            user_inputs.push_back(token);
        }
        return user_inputs;
    }

    /**
     * @brief Scans the provided paths to find all .txt files.
     * @param input_paths A vector of user-provided file or directory paths.
     * @return A vector of full paths to all found .txt files.
     */
    std::vector<std::string> find_txt_files_in_paths(const std::vector<std::string>& input_paths) {
        std::vector<std::string> files_to_process;
        for (const std::string& path_str : input_paths) {
            fs::path p(path_str);
            if (!fs::exists(p)) {
                std::cerr << "Warning: Path does not exist: " << path_str << std::endl;
                continue;
            }

            if (fs::is_regular_file(p) && p.extension() == ".txt") {
                files_to_process.push_back(p.string());
            } else if (fs::is_directory(p)) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(p)) {
                        if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".txt") {
                            files_to_process.push_back(entry.path().string());
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error accessing directory " << path_str << ": " << e.what() << std::endl;
                }
            }
        }
        return files_to_process;
    }
};


// =================================================================
// Refactored Class: FileProcessor
// =================================================================
/**
 * @class FileProcessor
 * @brief Orchestrates the file processing workflow.
 *
 * This class uses a FileFinder to get files, a DataFileParser to parse them,
 * and a DatabaseImporter to save the data. It also handles timing and reporting.
 */
class FileProcessor {
public:
    explicit FileProcessor(std::string db_name) : db_name_(std::move(db_name)) {}

    /**
     * @brief Runs the entire file processing pipeline.
     */
    void run() {
        FileFinder finder;
        std::vector<std::string> files_to_process = finder.collect_and_find_files();

        if (files_to_process.empty()) {
            std::cout << "No .txt files found to process." << std::endl;
            return;
        }

        std::cout << "\nStart processing " << files_to_process.size() << " file(s)... " << std::endl;
        auto start_total = std::chrono::high_resolution_clock::now();

        // Stage 1: Parsing
        std::cout << "Stage 1: Parsing files into memory..." << std::endl;
        DataFileParser parser;
        std::vector<std::string> failed_files;
        for (const std::string& fname : files_to_process) {
            if (!parser.parse_file(fname)) {
                failed_files.push_back(fname);
            }
        }
        parser.commit_all();
        auto end_parsing = std::chrono::high_resolution_clock::now();

        // Stage 2: Importing
        std::cout << "Stage 2: Importing data into the database..." << std::endl;
        DatabaseImporter importer(db_name_);
        if (!importer.is_db_open()) {
            std::cerr << "Importer could not open database. Aborting." << std::endl;
            double parsing_s = std::chrono::duration<double>(end_parsing - start_total).count();
            report_results(files_to_process.size(), failed_files, parsing_s, 0.0);
            return;
        }
        importer.import_data(parser);
        auto end_total = std::chrono::high_resolution_clock::now();

        // Calculate timings
        double parsing_s = std::chrono::duration<double>(end_parsing - start_total).count();
        double db_insertion_s = std::chrono::duration<double>(end_total - end_parsing).count();

        // Report final results
        report_results(files_to_process.size(), failed_files, parsing_s, db_insertion_s);
    }

private:
    /**
     * @brief Displays a summary report of the processing task.
     * @param total_file_count Total files attempted.
     * @param failed_files A list of files that failed to parse.
     * @param parsing_time Time taken for the parsing stage, in seconds.
     * @param db_time Time taken for the database import stage, in seconds.
     */
    void report_results(size_t total_file_count, const std::vector<std::string>& failed_files, double parsing_time, double db_time) {
        size_t successful_count = total_file_count - failed_files.size();
        double total_time = parsing_time + db_time;

        std::cout << "\n--- Data processing complete. ---" << std::endl;
        if (failed_files.empty()) {
            std::cout << ANSI_COLOR_GREEN << "All files successfully processed and imported." << ANSI_COLOR_RESET << std::endl;
            std::cout << "Successfully processed " << successful_count << " files." << std::endl;
        } else {
            std::cerr << "There were errors during the parsing stage." << std::endl;
            if (successful_count > 0) {
                std::cout << "Successfully parsed " << successful_count << " files." << std::endl;
            }
            std::cerr << "Failed to parse the following " << failed_files.size() << " files:" << std::endl;
            for (const std::string& fname : failed_files) {
                std::cerr << "- " << fname << std::endl;
            }
        }

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n--------------------------------------\n";
        std::cout << "Timing Statistics:\n" << std::endl;
        std::cout << "Total time: " << total_time << " seconds (" << total_time * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Parsing files: " << parsing_time << " seconds (" << parsing_time * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Database insertion: " << db_time << " seconds (" << db_time * 1000.0 << " ms)" << std::endl;
        std::cout << "--------------------------------------\n";
    }

    std::string db_name_;
};

// =================================================================
// Main Application Logic
// =================================================================

/**
 * @brief Simplified function to handle the file processing workflow.
 *
 * This function now instantiates and runs the FileProcessor class.
 */
void handle_process_files() {
    FileProcessor processor(DATABASE_NAME);
    processor.run();
}

bool open_database_if_needed(sqlite3*& db) {
    if (db == nullptr) {
        if (sqlite3_open(DATABASE_NAME.c_str(), &db)) {
            std::cerr << "Can't open database " << DATABASE_NAME << ": " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            db = nullptr;
            return false;
        }
    }
    return true;
}

// =================================================================
// Refactored Class: Menu
// =================================================================
/**
 * @class Menu
 * @brief Handles the main application menu and user interaction loop.
 */
class Menu {
public:
    Menu() : db(nullptr) {}

    /**
     * @brief Runs the main application loop.
     */
    void run() {
        while (true) {
            print_menu();
            int choice = -1;
            std::cin >> choice;

            if (std::cin.fail()) {
                std::cout << "Invalid input. Please enter a number." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            // Consume the rest of the line to handle trailing characters
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (!handle_user_choice(choice)) {
                break; // Exit loop if handler returns false
            }
        }

        if (db) {
            sqlite3_close(db);
        }
    }

private:
    sqlite3* db;

    void print_menu() {
        std::cout << "\n--- Time Tracking Menu ---" << std::endl;
        std::cout << "0. Process file(s) and import data" << std::endl;
        std::cout << "1. Query daily statistics" << std::endl;
        std::cout << "2. Query last 7 days" << std::endl;
        std::cout << "3. Query last 14 days" << std::endl;
        std::cout << "4. Query last 30 days" << std::endl;
        std::cout << "5. Generate study heatmap for a year" << std::endl;
        std::cout << "6. Query monthly statistics" << std::endl;
        std::cout << "7. Exit" << std::endl;
        std::cout << "Enter your choice: ";
    }

    std::string get_valid_date_input() {
        std::string date_str;
        while (true) {
            std::cout << "Enter date (YYYYMMDD): ";
            std::cin >> date_str;
            if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)) {
                int year = std::stoi(date_str.substr(0, 4));
                int month = std::stoi(date_str.substr(4, 2));
                int day = std::stoi(date_str.substr(6, 2));
                if (year > 1900 && year < 3000 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
                    break;
                }
            }
            std::cout << "Invalid date format or value. Please use YYYYMMDD." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        return date_str;
    }

    std::string get_valid_month_input() {
        std::string month_str;
        while (true) {
            std::cout << "Enter month (YYYYMM): ";
            std::cin >> month_str;
            if (month_str.length() == 6 && std::all_of(month_str.begin(), month_str.end(), ::isdigit)) {
                int year = std::stoi(month_str.substr(0, 4));
                int month = std::stoi(month_str.substr(4, 2));
                if (year > 1900 && year < 3000 && month >= 1 && month <= 12) {
                    break;
                }
            }
            std::cout << "Invalid month format or value. Please use YYYYMM." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        return month_str;
    }

    bool handle_user_choice(int choice) {
        // For any query, ensure the database is open
        if (choice >= 1 && choice <= 6) {
            if (!open_database_if_needed(db)) {
                return true; // Continue loop, but query will fail
            }
        }

        QueryHandler query_handler(db);

        switch (choice) {
            case 0:
                if (db) {
                    sqlite3_close(db); // Close DB connection before writing
                    db = nullptr;
                }
                handle_process_files();
                break;
            case 1: {
                std::string date_str = get_valid_date_input();
                query_handler.run_daily_query(date_str);
                break;
            }
            case 2:
                query_handler.run_period_query(7);
                break;
            case 3:
                query_handler.run_period_query(14);
                break;
            case 4:
                query_handler.run_period_query(30);
                break;
            case 5:
                std::cout << "\nFeature 'Generate study heatmap for a year' is not yet implemented." << std::endl;
                break;
            case 6: {
                std::string month_str = get_valid_month_input();
                query_handler.run_monthly_query(month_str);
                break;
            }
            case 7:
                std::cout << "Exiting program." << std::endl;
                return false; // Signal to exit
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
                break;
        }
        return true; // Signal to continue
    }
};


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

int main() {
    #if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    EnableVirtualTerminalProcessing();
    #endif

    Menu app_menu;
    app_menu.run();
    
    return 0;
}

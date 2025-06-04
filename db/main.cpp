#include <iostream>
#include <string>
#include <vector>
#include <limits> // Required for std::numeric_limits
#include <sqlite3.h>
#include "common_utils.h"   // Should be included first if others depend on its types
#include "data_parser.h"
#include "database_querier.h"

const std::string DATABASE_NAME = "time_data.db"; // Standardized DB name

void print_menu() {
    std::cout << "\n--- Time Tracking Menu ---" << std::endl;
    std::cout << "0. Process file(s) and import data" << std::endl;
    std::cout << "1. Query daily statistics" << std::endl;
    std::cout << "2. Query last 7 days" << std::endl;
    std::cout << "3. Query last 14 days" << std::endl;
    std::cout << "4. Query last 30 days" << std::endl; // Corrected numbering based on prompt
    std::cout << "5. Output raw data for a day" << std::endl;
    std::cout << "6. Generate study heatmap for a year" << std::endl;
    std::cout << "7. Query monthly statistics" << std::endl;
    std::cout << "8. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

// Helper to get validated YYYYMMDD date string
std::string get_valid_date_input() {
    std::string date_str;
    while (true) {
        std::cout << "Enter date (YYYYMMDD): ";
        std::cin >> date_str;
        if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)) {
            // Basic validation, could add month/day range checks
            int year = std::stoi(date_str.substr(0, 4));
            int month = std::stoi(date_str.substr(4, 2));
            int day = std::stoi(date_str.substr(6, 2));
            if (year > 1900 && year < 3000 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
                 break;
            }
        }
        std::cout << "Invalid date format or value. Please use YYYYMMDD." << std::endl;
        std::cin.clear(); // Clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
    }
    return date_str;
}

// Helper to get validated YYYYMM month string
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

// Helper to get validated YYYY year string
std::string get_valid_year_input() {
    std::string year_str;
    while (true) {
        std::cout << "Enter year (YYYY): ";
        std::cin >> year_str;
        if (year_str.length() == 4 && std::all_of(year_str.begin(), year_str.end(), ::isdigit)) {
            int year = std::stoi(year_str);
            if (year > 1900 && year < 3000) {
                break;
            }
        }
        std::cout << "Invalid year format or value. Please use YYYY." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return year_str;
}


int main() {
    sqlite3* db = nullptr; // Querier functions will use this directly
    int choice = -1;

    // --- UTF-8 Console Output Setup (Optional but good practice) ---
    // #ifdef _WIN32
    // #include <windows.h>
    // SetConsoleOutputCP(CP_UTF8);
    // SetConsoleCP(CP_UTF8);
    // #endif
    // std::ios_base::sync_with_stdio(false); // Can speed up C++ streams
    // std::cout.imbue(std::locale("")); // Use system's default locale for cout (might help with UTF-8)
    // std::cerr.imbue(std::locale("")); // Use system's default locale for cerr
    // ---

    while (choice != 8) {
        print_menu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1; // Reset choice
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Consume newline

        // Open database for querying if not already open, except for parsing
        if (choice != 0 && choice != 8 && db == nullptr) {
            int rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
            if (rc) {
                std::cerr << "Can't open database " << DATABASE_NAME << ": " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db); // It's safe to call sqlite3_close on a failed open if db is not NULL
                db = nullptr;      // Ensure db is NULL so we don't try to use it
                               // Choice will be re-evaluated, or user might pick 8 to exit.
            } else {
                std::cout << "Database " << DATABASE_NAME << " opened for querying." << std::endl;
            }
        }


        switch (choice) {
            case 0: {
                // For parsing, we use FileDataParser which manages its own DB connection.
                // Close the main 'db' if it's open, as FileDataParser will open its own.
                if (db) {
                    sqlite3_close(db);
                    db = nullptr;
                    std::cout << "Closed main DB connection for parsing." << std::endl;
                }
                std::cout << "Enter file name(s) to process (space-separated if multiple, then Enter): ";
                std::string line;
                std::getline(std::cin, line);
                std::stringstream ss(line);
                std::string filename;
                std::vector<std::string> files_to_process;
                while (ss >> filename) {
                    files_to_process.push_back(filename);
                }

                if (files_to_process.empty()) {
                    std::cout << "No filenames entered." << std::endl;
                    break;
                }
                
                FileDataParser parser(DATABASE_NAME); // Parser opens its own connection
                if (!parser.is_db_open()){
                    std::cerr << "Parser could not open database. Aborting file processing." << std::endl;
                    break;
                }
                for (const std::string& fname : files_to_process) {
                    std::cout << "\n--- Processing file: " << fname << " ---" << std::endl;
                    if (!parser.parse_file(fname)) {
                        std::cerr << "Failed to process file: " << fname << std::endl;
                    }
                }
                parser.commit_all(); // Ensure any final data is stored by the parser
                std::cout << "\n--- File processing complete. ---" << std::endl;
                break;
            }
            case 1: { // Query daily statistics
                if (!db) { std::cerr << "Database not open. Please process a file first or check DB connection." << std::endl; break; }
                std::string date_str = get_valid_date_input();
                query_day(db, date_str);
                break;
            }
            case 2: { // Query last 7 days
                 if (!db) { std::cerr << "Database not open." << std::endl; break; }
                query_period(db, 7);
                break;
            }
            case 3: { // Query last 14 days
                 if (!db) { std::cerr << "Database not open." << std::endl; break; }
                query_period(db, 14);
                break;
            }
            case 4: { // Query last 30 days
                 if (!db) { std::cerr << "Database not open." << std::endl; break; }
                query_period(db, 30);
                break;
            }
            case 5: { // Output raw data for a day
                 if (!db) { std::cerr << "Database not open." << std::endl; break; }
                std::string date_str = get_valid_date_input();
                query_day_raw(db, date_str);
                break;
            }
            case 6: { // Generate study heatmap for a year
                 if (!db) { std::cerr << "Database not open." << std::endl; break; }
                std::string year_str = get_valid_year_input();
                std::map<std::string, int> times = get_study_times(db, year_str);
                std::cout << "\n--- Daily Study Times for " << year_str << " (seconds) ---" << std::endl;
                if (times.empty()){
                    std::cout << "No study times found for " << year_str << "." << std::endl;
                } else {
                    // Sort by date for consistent output
                    std::vector<std::pair<std::string, int>> sorted_times(times.begin(), times.end());
                    std::sort(sorted_times.begin(), sorted_times.end());
                    for (const auto& pair : sorted_times) {
                        std::cout << pair.first << ": " << time_format_duration(pair.second) 
                                  << " (" << pair.second << "s)" << std::endl;
                    }
                }
                break;
            }
            case 7: { // Query monthly statistics
                 if (!db) { std::cerr << "Database not open." << std::endl; break; }
                std::string month_str = get_valid_month_input();
                query_month_summary(db, month_str);
                break;
            }
            case 8:
                std::cout << "Exiting program." << std::endl;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
                break;
        }
    }

    if (db) {
        sqlite3_close(db);
        std::cout << "Main database connection closed." << std::endl;
    }

    return 0;
}
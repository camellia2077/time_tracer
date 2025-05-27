#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <sqlite3.h>
#include <algorithm> // For std::sort, std::find_if
#include <iomanip>   // For std::setw, std::left

// --- Helper Structs and Functions ---

struct TimeEntry {
    std::string start_time_text;
    std::string end_time_text;
    std::string description_full;
    std::string parent_project;
    int duration_minutes;
};

// Helper function to trim leading/trailing whitespace
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return ""; // empty or all whitespace
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}


// Function to parse time HH:MM to minutes since midnight
int time_to_minutes(const std::string& time_str) {
    if (time_str.length() != 5 || time_str[2] != ':') {
        // Basic validation, though prompt says input is legal
        std::cerr << "Warning: Invalid time format encountered: " << time_str << std::endl;
        return 0; 
    }
    try {
        int hours = std::stoi(time_str.substr(0, 2));
        int minutes = std::stoi(time_str.substr(3, 2));
        return hours * 60 + minutes;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not parse time: " << time_str << " (" << e.what() << ")" << std::endl;
        return 0;
    }
}

// Function to calculate duration in minutes
int calculate_duration(const std::string& start_time_str, const std::string& end_time_str) {
    int start_mins = time_to_minutes(start_time_str);
    int end_mins = time_to_minutes(end_time_str);

    if (end_mins >= start_mins) {
        return end_mins - start_mins;
    } else { // Overnight
        return (24 * 60 - start_mins) + end_mins;
    }
}

std::string extract_parent_project(const std::string& description_full) {
    size_t underscore_pos = description_full.find('_');
    if (underscore_pos != std::string::npos) {
        return description_full.substr(0, underscore_pos);
    }
    return description_full; // If no underscore, the whole thing is the parent
}

// --- Database Functions ---

void create_tables(sqlite3* db) {
    char* err_msg = nullptr;
    const char* sql_daily_summaries =
        "CREATE TABLE IF NOT EXISTS DailySummaries ("
        "record_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date_text TEXT UNIQUE NOT NULL,"
        "status INTEGER NOT NULL,"            // 0 for False, 1 for True
        "getup_time TEXT,"
        "remark TEXT"
        ");";

    const char* sql_time_entries =
        "CREATE TABLE IF NOT EXISTS TimeEntries ("
        "entry_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "record_id INTEGER NOT NULL,"
        "start_time_text TEXT NOT NULL,"
        "end_time_text TEXT NOT NULL,"
        "description_full TEXT NOT NULL,"
        "parent_project TEXT NOT NULL,"
        "duration_minutes INTEGER NOT NULL,"
        "FOREIGN KEY (record_id) REFERENCES DailySummaries(record_id) ON DELETE CASCADE" // Added ON DELETE CASCADE
        ");";

    if (sqlite3_exec(db, sql_daily_summaries, 0, 0, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error (Create DailySummaries): " << err_msg << std::endl;
        sqlite3_free(err_msg);
    } else {
        // std::cout << "DailySummaries table checked/created successfully." << std::endl;
    }

    if (sqlite3_exec(db, sql_time_entries, 0, 0, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error (Create TimeEntries): " << err_msg << std::endl;
        sqlite3_free(err_msg);
    } else {
        // std::cout << "TimeEntries table checked/created successfully." << std::endl;
    }
}

void process_file(sqlite3* db, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << std::endl;
        return;
    }

    std::string line;
    std::string current_date;
    bool current_status_val = false;
    std::string current_getup_time;
    std::string current_remark_text;
    std::vector<TimeEntry> current_entries;

    auto save_current_day_data = [&]() {
        if (current_date.empty()) return;

        char* err_msg_transaction = nullptr;
        sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg_transaction);

        sqlite3_stmt* stmt_find_date = nullptr;
        long long record_id = -1;

        // Check if date exists
        std::string sql_find = "SELECT record_id FROM DailySummaries WHERE date_text = ?;";
        if (sqlite3_prepare_v2(db, sql_find.c_str(), -1, &stmt_find_date, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt_find_date, 1, current_date.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt_find_date) == SQLITE_ROW) {
                record_id = sqlite3_column_int64(stmt_find_date, 0);
                // Date exists, delete old time entries. DailySummary will be updated.
                // ON DELETE CASCADE on FOREIGN KEY in TimeEntries will handle this if we delete from DailySummaries
                // Or, more explicitly:
                sqlite3_stmt* stmt_delete_entries = nullptr;
                std::string sql_delete = "DELETE FROM TimeEntries WHERE record_id = ?;";
                if (sqlite3_prepare_v2(db, sql_delete.c_str(), -1, &stmt_delete_entries, nullptr) == SQLITE_OK) {
                    sqlite3_bind_int64(stmt_delete_entries, 1, record_id);
                    if (sqlite3_step(stmt_delete_entries) != SQLITE_DONE) {
                        std::cerr << "Error deleting old time entries for date " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
                    }
                    sqlite3_finalize(stmt_delete_entries);
                } else {
                     std::cerr << "Error preparing delete statement: " << sqlite3_errmsg(db) << std::endl;
                }
            }
            sqlite3_finalize(stmt_find_date);
        } else {
            std::cerr << "Error preparing find_date statement: " << sqlite3_errmsg(db) << std::endl;
        }


        if (record_id != -1) { // Update existing DailySummary
            sqlite3_stmt* stmt_update_daily = nullptr;
            std::string sql_update = "UPDATE DailySummaries SET status = ?, getup_time = ?, remark = ? WHERE record_id = ?;";
            if (sqlite3_prepare_v2(db, sql_update.c_str(), -1, &stmt_update_daily, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt_update_daily, 1, current_status_val ? 1 : 0);
                sqlite3_bind_text(stmt_update_daily, 2, current_getup_time.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt_update_daily, 3, current_remark_text.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int64(stmt_update_daily, 4, record_id);
                if (sqlite3_step(stmt_update_daily) != SQLITE_DONE) {
                    std::cerr << "Error updating DailySummaries for date " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt_update_daily);
            } else {
                 std::cerr << "Error preparing update daily_summary statement: " << sqlite3_errmsg(db) << std::endl;
            }
        } else { // Insert new DailySummary
            sqlite3_stmt* stmt_insert_daily = nullptr;
            std::string sql_insert = "INSERT INTO DailySummaries (date_text, status, getup_time, remark) VALUES (?, ?, ?, ?);";
            if (sqlite3_prepare_v2(db, sql_insert.c_str(), -1, &stmt_insert_daily, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt_insert_daily, 1, current_date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt_insert_daily, 2, current_status_val ? 1 : 0);
                sqlite3_bind_text(stmt_insert_daily, 3, current_getup_time.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt_insert_daily, 4, current_remark_text.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(stmt_insert_daily) == SQLITE_DONE) {
                    record_id = sqlite3_last_insert_rowid(db);
                } else {
                    std::cerr << "Error inserting DailySummaries for date " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt_insert_daily);
            } else {
                 std::cerr << "Error preparing insert daily_summary statement: " << sqlite3_errmsg(db) << std::endl;
            }
        }

        // Insert TimeEntries
        if (record_id != -1) {
            sqlite3_stmt* stmt_insert_entry = nullptr;
            std::string sql_insert_entry = "INSERT INTO TimeEntries (record_id, start_time_text, end_time_text, description_full, parent_project, duration_minutes) VALUES (?, ?, ?, ?, ?, ?);";
            if (sqlite3_prepare_v2(db, sql_insert_entry.c_str(), -1, &stmt_insert_entry, nullptr) == SQLITE_OK) {
                for (const auto& entry : current_entries) {
                    sqlite3_bind_int64(stmt_insert_entry, 1, record_id);
                    sqlite3_bind_text(stmt_insert_entry, 2, entry.start_time_text.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert_entry, 3, entry.end_time_text.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert_entry, 4, entry.description_full.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert_entry, 5, entry.parent_project.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt_insert_entry, 6, entry.duration_minutes);
                    if (sqlite3_step(stmt_insert_entry) != SQLITE_DONE) {
                        std::cerr << "Error inserting time entry for " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
                    }
                    sqlite3_reset(stmt_insert_entry); // Reset for next iteration
                }
                sqlite3_finalize(stmt_insert_entry);
            } else {
                std::cerr << "Error preparing insert_entry statement: " << sqlite3_errmsg(db) << std::endl;
            }
        }
        
        if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err_msg_transaction) != SQLITE_OK) {
             std::cerr << "SQL error (Commit): " << err_msg_transaction << std::endl;
             sqlite3_free(err_msg_transaction);
             sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); // Attempt rollback
        }


        // Clear for next day block
        current_date.clear();
        current_status_val = false;
        current_getup_time.clear();
        current_remark_text.clear();
        current_entries.clear();
    };

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line.rfind("Date:", 0) == 0) {
            save_current_day_data(); // Save previous day's data first
            current_date = trim(line.substr(5));
        } else if (line.rfind("Status:", 0) == 0) {
            std::string status_str = trim(line.substr(7));
            current_status_val = (status_str == "True" || status_str == "true");
        } else if (line.rfind("Getup:", 0) == 0) {
            current_getup_time = trim(line.substr(6));
        } else if (line.rfind("Remark:", 0) == 0) {
            current_remark_text = trim(line.substr(7));
        } else {
            // Attempt to parse as time entry: HH:MM~HH:MMdescription
            size_t tilde_pos = line.find('~');
            if (tilde_pos != std::string::npos && tilde_pos > 4 && line.length() > tilde_pos + 5) {
                 // Check for HH:MM format more carefully
                bool is_time_entry_format = true;
                if (line[tilde_pos-3] != ':' || line[tilde_pos+3] != ':') {
                    is_time_entry_format = false;
                }
                // A simple check to see if it resembles a time entry
                if (is_time_entry_format && isdigit(line[0]) && isdigit(line[1]) && line[2] == ':' && 
                    isdigit(line[3]) && isdigit(line[4]) &&
                    isdigit(line[tilde_pos+1]) && isdigit(line[tilde_pos+2]) && line[tilde_pos+3] == ':' &&
                    isdigit(line[tilde_pos+4]) && isdigit(line[tilde_pos+5]))
                {
                    TimeEntry entry;
                    entry.start_time_text = line.substr(0, 5);
                    entry.end_time_text = line.substr(tilde_pos + 1, 5);
                    entry.description_full = trim(line.substr(tilde_pos + 1 + 5));
                    entry.parent_project = extract_parent_project(entry.description_full);
                    entry.duration_minutes = calculate_duration(entry.start_time_text, entry.end_time_text);
                    current_entries.push_back(entry);
                } else {
                    // Could be part of a multi-line remark if not strictly one line
                    // For now, assume remarks are single line as per example structure.
                    // If current_remark_text is not empty, and we are in a date block, append.
                    // However, the problem structure implies remarks are one-liners.
                }
            }
        }
    }
    save_current_day_data(); // Save the last day's data

    std::cout << "File processing complete. Data stored/updated in database." << std::endl;
    file.close();
}

void query_data(sqlite3* db, const std::string& date_query) {
    long long record_id = -1;
    sqlite3_stmt* stmt_find_date = nullptr;

    std::string sql_find_id = "SELECT record_id FROM DailySummaries WHERE date_text = ?;";
    if (sqlite3_prepare_v2(db, sql_find_id.c_str(), -1, &stmt_find_date, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt_find_date, 1, date_query.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt_find_date) == SQLITE_ROW) {
            record_id = sqlite3_column_int64(stmt_find_date, 0);
        } else {
            std::cout << "No data found for date: " << date_query << std::endl;
            sqlite3_finalize(stmt_find_date);
            return;
        }
        sqlite3_finalize(stmt_find_date);
    } else {
        std::cerr << "SQL error (preparing to find date for query): " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    if (record_id == -1) return; // Should have been caught above

    std::cout << "Query results for date: " << date_query << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    sqlite3_stmt* stmt_parent_totals = nullptr;
    std::string sql_parent_totals =
        "SELECT parent_project, SUM(duration_minutes) as total_duration "
        "FROM TimeEntries "
        "WHERE record_id = ? "
        "GROUP BY parent_project "
        "ORDER BY total_duration DESC;";

    if (sqlite3_prepare_v2(db, sql_parent_totals.c_str(), -1, &stmt_parent_totals, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt_parent_totals, 1, record_id);

        bool has_results = false;
        while (sqlite3_step(stmt_parent_totals) == SQLITE_ROW) {
            has_results = true;
            std::string parent_project = reinterpret_cast<const char*>(sqlite3_column_text(stmt_parent_totals, 0));
            int total_duration = sqlite3_column_int(stmt_parent_totals, 1);

            std::cout << parent_project << " (Total: " << total_duration << " minutes)" << std::endl;

            sqlite3_stmt* stmt_child_entries = nullptr;
            std::string sql_child_entries =
                "SELECT description_full, duration_minutes, start_time_text "
                "FROM TimeEntries "
                "WHERE record_id = ? AND parent_project = ? "
                "ORDER BY start_time_text;"; // Order by start time for clarity

            if (sqlite3_prepare_v2(db, sql_child_entries.c_str(), -1, &stmt_child_entries, nullptr) == SQLITE_OK) {
                sqlite3_bind_int64(stmt_child_entries, 1, record_id);
                sqlite3_bind_text(stmt_child_entries, 2, parent_project.c_str(), -1, SQLITE_STATIC);

                while (sqlite3_step(stmt_child_entries) == SQLITE_ROW) {
                    std::string desc_full = reinterpret_cast<const char*>(sqlite3_column_text(stmt_child_entries, 0));
                    int duration = sqlite3_column_int(stmt_child_entries, 1);
                    // std::string start_t = reinterpret_cast<const char*>(sqlite3_column_text(stmt_child_entries, 2));
                    std::cout << "  - " << std::left << std::setw(40) << desc_full 
                              << " (" << duration << " minutes)" << std::endl;
                }
                sqlite3_finalize(stmt_child_entries);
            } else {
                std::cerr << "SQL error (preparing child entries query): " << sqlite3_errmsg(db) << std::endl;
            }
            std::cout << std::endl; // Add a blank line between parent projects
        }
        if (!has_results) {
             std::cout << "No time entries found for this date, although a daily summary exists." << std::endl;
        }
        sqlite3_finalize(stmt_parent_totals);

    } else {
        std::cerr << "SQL error (preparing parent totals query): " << sqlite3_errmsg(db) << std::endl;
    }
     std::cout << "------------------------------------------" << std::endl;
}


// --- Main Function ---

int main(int argc, char* argv[]) {
    // Ensure console can display Chinese characters (example for Windows, might need adjustment)
    // system("chcp 65001 > nul"); // Sets console to UTF-8 on Windows. Comment out if not needed/problematic.
    // std::locale::global(std::locale("")); // For broader locale support with wcout if used

    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " <filepath.txt>      (To process and store data from the text file)\n"
                  << "  " << argv[0] << " query <YYYYMMDD>  (To query data for a specific date)\n";
        return 1;
    }

    sqlite3* db;
    const char* db_file = "TimeManagement.db";
    int rc = sqlite3_open(db_file, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        // std::cout << "Opened database successfully: " << db_file << std::endl;
    }

    create_tables(db);

    std::string first_arg = argv[1];

    if (first_arg == "query") {
        if (argc == 3) {
            std::string date_to_query = argv[2];
            // Basic validation for YYYYMMDD format
            if (date_to_query.length() == 8 && std::all_of(date_to_query.begin(), date_to_query.end(), ::isdigit)) {
                 query_data(db, date_to_query);
            } else {
                std::cerr << "Error: Date for query must be in YYYYMMDD format (e.g., 20250525)." << std::endl;
            }
        } else {
            std::cerr << "Usage: " << argv[0] << " query <YYYYMMDD>\n";
        }
    } else { // Assume it's a filepath
        if (argc == 2) {
            process_file(db, first_arg);
        } else {
            std::cerr << "Invalid arguments for file processing.\n"
                      << "Usage: " << argv[0] << " <filepath.txt>\n";
        }
    }

    sqlite3_close(db);
    // std::cout << "Database connection closed." << std::endl;
    return 0;
}

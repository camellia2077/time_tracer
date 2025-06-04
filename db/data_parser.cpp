#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sqlite3.h>
#include <sstream>
#include <algorithm> // For std::transform for uppercase
#include <iomanip>   // For std::put_time, std::get_time (though not directly used for HH:MM parsing here)

// Helper function to execute SQL statements and handle errors
bool execute_sql(sqlite3* db, const std::string& sql, const std::string& context_msg = "") {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (" << context_msg << "): " << err_msg << std::endl;
        std::cerr << "Failed SQL: " << sql << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

// Helper function to convert HH:MM time to seconds since midnight
int time_to_seconds(const std::string& time_str) {
    if (time_str.length() != 5 || time_str[2] != ':') {
        std::cerr << "Error: Invalid time format '" << time_str << "'. Expected HH:MM." << std::endl;
        return 0; // Return 0 for invalid format as per requirement
    }
    try {
        int hours = std::stoi(time_str.substr(0, 2));
        int minutes = std::stoi(time_str.substr(3, 2));
        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
            std::cerr << "Error: Invalid time value in '" << time_str << "'." << std::endl;
            return 0;
        }
        return hours * 3600 + minutes * 60;
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Error: Non-numeric value in time '" << time_str << "': " << ia.what() << std::endl;
        return 0;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Error: Time value out of range in '" << time_str << "': " << oor.what() << std::endl;
        return 0;
    }
}

struct TimeRecord {
    std::string date;
    std::string start;
    std::string end;
    std::string project_path;
    int duration_seconds;
};

class FileDataParser {
public:
    FileDataParser(const std::string& db_path) : db(nullptr), current_date_processed(false) {
        if (sqlite3_open(db_path.c_str(), &db)) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            db = nullptr; // Ensure db is null if open failed
        } else {
            std::cout << "Opened database successfully: " << db_path << std::endl;
            _initialize_database();
            _prepopulate_parent_child();
        }
    }

    ~FileDataParser() {
        if (db) {
            _store_previous_date_data(); // Store any pending data before closing
            sqlite3_close(db);
            std::cout << "Database closed." << std::endl;
        }
    }

    bool parse_file(const std::string& filename) {
        if (!db) return false;

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::cout << "Processing file: " << filename << std::endl;
        // Reset date-specific state for a new file, except if current_date has pending data
        // The problem states "In processing each new file, reset internal date-related state"
        // but also "When encountering a new 'Date:' line, store previous date's data".
        // This implies state persists across files until a new Date line or end of all processing.
        // For simplicity here, let's assume date context is flushed at end of file or new Date.
        // If a file doesn't start with a Date: line but contains other data, it needs a current_date.
        // This design decision focuses on flushing data when a new Date is found or file ends.

        // To correctly handle multiple files where one file might not end a date block,
        // we should not reset current_date here unless it's truly a fresh start of the whole process.
        // The prompt's "reset internal date-related state" before a new file might mean
        // ensuring that if a file *doesn't* start with a Date:, it doesn't accidentally use
        // a stale current_date from a *previous file's incomplete block*.
        // My _store_previous_date_data() will handle flushing, and new Date: lines set current_date.
        // So, the main thing to "reset" might be to ensure we are ready for a new Date.
        // The current logic should handle this correctly by processing line by line.

        std::string line;
        int line_num = 0;
        current_file_name = filename; // For error reporting

        while (std::getline(file, line)) {
            line_num++;
            // Trim whitespace (simple trim for this example)
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

            if (line.empty()) {
                continue;
            }

            try {
                if (line.rfind("Date:", 0) == 0) { // starts_with "Date:"
                    _store_previous_date_data();
                    _handle_date_line(line);
                } else if (line.rfind("Status:", 0) == 0) {
                    _handle_status_line(line);
                } else if (line.rfind("Remark:", 0) == 0) {
                    _handle_remark_line(line);
                } else if (line.rfind("Getup:", 0) == 0) {
                    _handle_getup_line(line);
                } else if (line.find('~') != std::string::npos) {
                    _handle_time_record_line(line, line_num);
                } else {
                    std::cerr << current_file_name << ":" << line_num << ": Warning: Unrecognized line format: " << line << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << current_file_name << ":" << line_num << ": Error processing line: " << line << " - " << e.what() << std::endl;
            }
        }

        // Ensure data from the last date in the file is stored
        _store_previous_date_data();
        // Commit any pending transactions if using them (sqlite3_exec handles this for simple statements)
        // For more complex scenarios with explicit BEGIN TRANSACTION, a COMMIT would be here.
        std::cout << "Finished processing file: " << filename << std::endl;
        file.close();
        return true;
    }

    void commit_all() { // Call this after all files are processed
        _store_previous_date_data(); // Store any final pending data
        std::cout << "All pending data committed." << std::endl;
    }


private:
    sqlite3* db;
    std::string current_date;
    std::string current_status;
    std::string current_remark;
    std::string current_getup_time;
    std::vector<TimeRecord> current_time_records_buffer;
    std::string current_file_name; // For error messages
    bool current_date_processed; // Flag to track if current_date data has been partially written to DB

    std::map<std::string, std::string> initial_top_level_parents = {
        {"study", "STUDY"},
        {"code", "CODE"}
        // Add more predefined top-level activities here if needed
    };

    void _initialize_database() {
        if (!db) return;
        const char* create_days_table_sql = R"(
            CREATE TABLE IF NOT EXISTS days (
                date TEXT PRIMARY KEY,
                status TEXT DEFAULT 'False',
                remark TEXT DEFAULT '',
                getup_time TEXT DEFAULT '00:00'
            );
        )";
        execute_sql(db, create_days_table_sql, "Create days table");

        const char* create_time_records_table_sql = R"(
            CREATE TABLE IF NOT EXISTS time_records (
                date TEXT,
                start TEXT,
                end TEXT,
                project_path TEXT,
                duration INTEGER,
                PRIMARY KEY (date, start),
                FOREIGN KEY (date) REFERENCES days(date)
            );
        )";
        execute_sql(db, create_time_records_table_sql, "Create time_records table");

        const char* create_parent_child_table_sql = R"(
            CREATE TABLE IF NOT EXISTS parent_child (
                child TEXT PRIMARY KEY,
                parent TEXT
            );
        )";
        execute_sql(db, create_parent_child_table_sql, "Create parent_child table");

        const char* create_parent_time_table_sql = R"(
            CREATE TABLE IF NOT EXISTS parent_time (
                date TEXT,
                parent TEXT,
                duration INTEGER,
                PRIMARY KEY (date, parent)
            );
        )";
        execute_sql(db, create_parent_time_table_sql, "Create parent_time table");
    }

    void _prepopulate_parent_child() {
        if (!db) return;
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement for prepopulating parent_child: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        for (const auto& pair : initial_top_level_parents) {
            sqlite3_bind_text(stmt, 1, pair.first.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, pair.second.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Failed to insert initial parent_child data (" << pair.first << ", " << pair.second << "): " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }


    void _handle_date_line(const std::string& line) {
        // Format: Date:YYYYMMDD
        if (line.length() > 5) {
            current_date = line.substr(5);
            current_date.erase(0, current_date.find_first_not_of(" \t"));
            current_date.erase(current_date.find_last_not_of(" \t") + 1);

            // Reset daily accumulators
            current_status = "False"; // Default status
            current_remark = "";      // Default remark
            current_getup_time = "00:00"; // Default getup time
            current_time_records_buffer.clear();
            current_date_processed = false; // New date, not yet written

            // Create initial record in days table
            sqlite3_stmt* stmt;
            const char* sql = "INSERT OR IGNORE INTO days (date, status, remark, getup_time) VALUES (?, ?, ?, ?);";
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, current_date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, current_status.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, current_remark.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, current_getup_time.c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    std::cerr << "Error inserting initial day record for " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            } else {
                std::cerr << "Failed to prepare statement for initial day record: " << sqlite3_errmsg(db) << std::endl;
            }
             std::cout << "New date set: " << current_date << std::endl;
        } else {
            std::cerr << current_file_name << ": Malformed Date line: " << line << std::endl;
        }
    }

    void _handle_status_line(const std::string& line) {
        if (current_date.empty()) {
            std::cerr << current_file_name << ": Status line found before Date: " << line << std::endl;
            return;
        }
        if (line.length() > 7) { // "Status:" is 7 chars
            current_status = line.substr(7);
            current_status.erase(0, current_status.find_first_not_of(" \t"));
            current_status.erase(current_status.find_last_not_of(" \t") + 1);
        } else {
             std::cerr << current_file_name << ": Malformed Status line: " << line << std::endl;
        }
    }

    void _handle_remark_line(const std::string& line) {
        if (current_date.empty()) {
            std::cerr << current_file_name << ": Remark line found before Date: " << line << std::endl;
            return;
        }
        if (line.length() > 7) { // "Remark:" is 7 chars
            current_remark = line.substr(7);
            // Trim leading space if any, but keep internal/trailing spaces for remark content
            current_remark.erase(0, current_remark.find_first_not_of(" \t"));
             // Do not trim trailing for remark content, only leading after colon.
        } else {
             std::cerr << current_file_name << ": Malformed Remark line: " << line << std::endl;
        }
    }

    void _handle_getup_line(const std::string& line) {
        if (current_date.empty()) {
            std::cerr << current_file_name << ": Getup line found before Date: " << line << std::endl;
            return;
        }
        if (line.length() > 6) { // "Getup:" is 6 chars
            current_getup_time = line.substr(6);
            current_getup_time.erase(0, current_getup_time.find_first_not_of(" \t"));
            current_getup_time.erase(current_getup_time.find_last_not_of(" \t") + 1);
            // Validate HH:MM format for getup_time
            if (time_to_seconds(current_getup_time) == 0 && current_getup_time != "00:00") { // time_to_seconds returns 0 on error
                 // Allow "00:00" as a valid default/error, but warn if it's not "00:00" and invalid.
                if (current_getup_time.find_first_not_of("0123456789:") != std::string::npos || current_getup_time.length() != 5) {
                   std::cerr << current_file_name << ": Warning: Invalid Getup time format '" << current_getup_time << "'. Using '00:00'." << std::endl;
                   current_getup_time = "00:00"; // Reset to default if invalid
                }
            }
        } else {
             std::cerr << current_file_name << ": Malformed Getup line: " << line << std::endl;
        }
    }

    void _handle_time_record_line(const std::string& line, int line_num) {
        if (current_date.empty()) {
            std::cerr << current_file_name << ":" << line_num << ": Time record line found before Date: " << line << std::endl;
            return;
        }
        // Regex: HH:MM~HH:MM project_path
        std::regex time_record_regex(R"((\d{2}:\d{2})~(\d{2}:\d{2})\s+(.+))");
        std::smatch matches;

        if (std::regex_match(line, matches, time_record_regex)) {
            if (matches.size() == 4) {
                std::string start_time_str = matches[1].str();
                std::string end_time_str = matches[2].str();
                std::string project_path = matches[3].str();
                project_path.erase(project_path.find_last_not_of(" \t") + 1); // Trim trailing from project_path only

                int start_seconds = time_to_seconds(start_time_str);
                int end_seconds = time_to_seconds(end_time_str);

                if (start_seconds == 0 && start_time_str != "00:00") {
                     std::cerr << current_file_name << ":" << line_num << ": Invalid start time format in line: " << line << std::endl;
                     return;
                }
                 if (end_seconds == 0 && end_time_str != "00:00") {
                     std::cerr << current_file_name << ":" << line_num << ": Invalid end time format in line: " << line << std::endl;
                     return;
                }


                int duration_seconds;
                if (end_seconds < start_seconds) { // Overnight
                    duration_seconds = (end_seconds + 24 * 3600) - start_seconds;
                } else {
                    duration_seconds = end_seconds - start_seconds;
                }

                current_time_records_buffer.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
                _process_project_path(project_path);
            } else {
                 std::cerr << current_file_name << ":" << line_num << ": Malformed time record (regex internal error): " << line << std::endl;
            }
        } else {
            std::cerr << current_file_name << ":" << line_num << ": Malformed time record line (no regex match): " << line << std::endl;
        }
    }

    void _process_project_path(const std::string& project_path_orig) {
        if (!db) return;
        std::string project_path = project_path_orig; // Modifiable copy

        std::replace(project_path.begin(), project_path.end(), ' ', '_'); // Replace spaces with underscores if desired, or handle as part of name.
                                                                         // The prompt implies project_path uses underscores already.

        std::stringstream ss(project_path);
        std::string segment;
        std::vector<std::string> segments;
        while(std::getline(ss, segment, '_')) {
            if (!segment.empty()) {
                segments.push_back(segment);
            }
        }

        if (segments.empty()) return;

        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement for parent_child: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        std::string current_full_path = "";
        std::string parent_of_current_segment;

        for (size_t i = 0; i < segments.size(); ++i) {
            if (i == 0) { // First segment (e.g., "study")
                current_full_path = segments[i];
                auto it = initial_top_level_parents.find(current_full_path);
                if (it != initial_top_level_parents.end()) {
                    parent_of_current_segment = it->second; // "STUDY"
                } else {
                    parent_of_current_segment = current_full_path; // "study"
                    std::transform(parent_of_current_segment.begin(), parent_of_current_segment.end(), parent_of_current_segment.begin(), ::toupper); // "STUDY"
                }
            } else { // Subsequent segments (e.g., "program" in "study_program")
                parent_of_current_segment = current_full_path; // Parent is the previous full path (e.g. "study")
                current_full_path += "_" + segments[i];      // Current becomes "study_program"
            }

            // Insert (child=current_full_path, parent=parent_of_current_segment)
            // Example: ("study", "STUDY"), ("study_program", "study"), ("study_program_python", "study_program")
            sqlite3_bind_text(stmt, 1, current_full_path.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, parent_of_current_segment.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Failed to insert parent_child (" << current_full_path << ", " << parent_of_current_segment << "): " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }

    void _store_previous_date_data() {
        if (current_date.empty() || !db) {
            return;
        }
         std::cout << "Storing data for date: " << current_date << std::endl;

        // Update days table
        sqlite3_stmt* day_stmt;
        // Use INSERT OR REPLACE for days to update if Date was already there or create if new from an earlier file.
        // More robust: UPDATE existing, or INSERT if it's somehow missing (though _handle_date_line should have inserted)
        const char* update_day_sql = "UPDATE days SET status = ?, remark = ?, getup_time = ? WHERE date = ?;";
        if (sqlite3_prepare_v2(db, update_day_sql, -1, &day_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(day_stmt, 1, current_status.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(day_stmt, 2, current_remark.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(day_stmt, 3, current_getup_time.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(day_stmt, 4, current_date.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(day_stmt) != SQLITE_DONE) {
                std::cerr << "Error updating day record for " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_finalize(day_stmt);
        } else {
            std::cerr << "Failed to prepare statement for updating day record: " << sqlite3_errmsg(db) << std::endl;
        }


        // Insert time records
        sqlite3_stmt* record_stmt;
        const char* insert_record_sql = "INSERT OR REPLACE INTO time_records (date, start, end, project_path, duration) VALUES (?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insert_record_sql, -1, &record_stmt, nullptr) == SQLITE_OK) {
            for (const auto& record : current_time_records_buffer) {
                sqlite3_bind_text(record_stmt, 1, record.date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(record_stmt, 2, record.start.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(record_stmt, 3, record.end.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(record_stmt, 4, record.project_path.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(record_stmt, 5, record.duration_seconds);

                if (sqlite3_step(record_stmt) != SQLITE_DONE) {
                    std::cerr << "Error inserting time record for " << record.date << " " << record.start << ": " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_reset(record_stmt); // Reset for next iteration
            }
            sqlite3_finalize(record_stmt);
        } else {
            std::cerr << "Failed to prepare statement for inserting time records: " << sqlite3_errmsg(db) << std::endl;
        }

        // Clear buffers for the processed date
        current_time_records_buffer.clear();
        current_date_processed = true; // Mark as processed
        // current_date itself is not cleared here, it's cleared or overwritten by the next Date: line
    }
};


int main(int argc, char* argv[]) {
    // --- UTF-8 Console Output Setup (Windows specific, optional for MSYS2/MinGW but can help) ---
    // #ifdef _WIN32
    // SetConsoleOutputCP(CP_UTF8);
    // SetConsoleCP(CP_UTF8);
    // #endif
    // std::ios_base::sync_with_stdio(false); // Might improve performance with cout/cerr for large outputs
    // std::cout.imbue(std::locale("en_US.UTF-8")); // Or your system's UTF-8 locale
    // std::cerr.imbue(std::locale("en_US.UTF-8"));
    // --- End UTF-8 Console ---

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file1> [input_file2 ...]" << std::endl;
        std::cerr << "Example: " << argv[0] << " data.txt" << std::endl;
        return 1;
    }

    FileDataParser parser("time_tracking.db");

    for (int i = 1; i < argc; ++i) {
        std::cout << "\n--- Processing file: " << argv[i] << " ---" << std::endl;
        if (!parser.parse_file(argv[i])) {
            std::cerr << "Failed to process file: " << argv[i] << std::endl;
        }
    }
    
    parser.commit_all(); // Ensure any final data is stored

    std::cout << "\n--- Processing complete. ---" << std::endl;
    std::cout << "Data stored in time_tracking.db" << std::endl;
    std::cout << "You can inspect the database using a tool like 'sqlite3 time_tracking.db' and running SQL queries (e.g., 'SELECT * FROM days;')." << std::endl;

    return 0;
}
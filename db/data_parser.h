#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sqlite3.h>
#include <sstream>
#include <algorithm> // For std::transform
#include <map>
#include "common_utils.h" // For time_str_to_seconds

// Helper function to execute SQL statements and handle errors
inline bool execute_sql_parser(sqlite3* db, const std::string& sql, const std::string& context_msg = "") {
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

struct TimeRecordInternal {
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
            db = nullptr;
        } else {
            _initialize_database();
            _prepopulate_parent_child();
        }
    }

    ~FileDataParser() {
        if (db) {
            // Ensure the last buffered data is stored before closing.
            _store_previous_date_data();
            sqlite3_close(db);
        }
    }

    bool is_db_open() const {
        return db != nullptr;
    }

    bool parse_file(const std::string& filename) {
        if (!db) return false;

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string line;
        int line_num = 0;
        current_file_name = filename;

        while (std::getline(file, line)) {
            line_num++;
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

            if (line.empty()) {
                continue;
            }

            try {
                if (line.rfind("Date:", 0) == 0) {
                    // When we find a new date, store all data for the previous one.
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
        // After the loop, store the data for the very last date in the file.
        _store_previous_date_data();
        file.close();
        return true;
    }
    
    // Public method to explicitly commit any pending data.
    void commit_all() {
        _store_previous_date_data();
    }


private:
    sqlite3* db;
    std::string current_date;
    std::string current_status;
    std::string current_remark;
    std::string current_getup_time;
    std::vector<TimeRecordInternal> current_time_records_buffer;
    std::string current_file_name;
    bool current_date_processed;

    std::map<std::string, std::string> initial_top_level_parents = {
        {"study", "STUDY"},
        {"code", "CODE"}
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
        execute_sql_parser(db, create_days_table_sql, "Create days table");

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
        execute_sql_parser(db, create_time_records_table_sql, "Create time_records table");

        const char* create_parent_child_table_sql = R"(
            CREATE TABLE IF NOT EXISTS parent_child (
                child TEXT PRIMARY KEY,
                parent TEXT
            );
        )";
        execute_sql_parser(db, create_parent_child_table_sql, "Create parent_child table");
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
                if (sqlite3_errcode(db) != SQLITE_CONSTRAINT_PRIMARYKEY && sqlite3_errcode(db) != SQLITE_CONSTRAINT_UNIQUE) {
                     std::cerr << "Failed to insert initial parent_child data (" << pair.first << ", " << pair.second << "): " << sqlite3_errmsg(db) << std::endl;
                }
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }


    void _handle_date_line(const std::string& line) {
        if (line.length() > 5) {
            current_date = line.substr(5);
            current_date.erase(0, current_date.find_first_not_of(" \t"));
            current_date.erase(current_date.find_last_not_of(" \t") + 1);

            // Reset buffers for the new date
            current_status = "False";
            current_remark = "";
            current_getup_time = "00:00";
            current_time_records_buffer.clear();
            current_date_processed = false;

            // Insert the initial record for the day. This is done outside the main transaction
            // as it's a prerequisite for the foreign key constraints in time_records.
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
        } else {
            std::cerr << current_file_name << ": Malformed Date line: " << line << std::endl;
        }
    }

    void _handle_status_line(const std::string& line) {
        if (current_date.empty()) {
            std::cerr << current_file_name << ": Status line found before Date: " << line << std::endl;
            return;
        }
        if (line.length() > 7) { 
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
        if (line.length() >= 7) { 
            if (line.length() > 7) {
                 current_remark = line.substr(7);
                 current_remark.erase(0, current_remark.find_first_not_of(" \t"));
            } else {
                current_remark = "";
            }
        } else {
             std::cerr << current_file_name << ": Malformed Remark line (too short): " << line << std::endl;
        }
    }

    void _handle_getup_line(const std::string& line) {
        if (current_date.empty()) {
            std::cerr << current_file_name << ": Getup line found before Date: " << line << std::endl;
            return;
        }
        if (line.length() > 6) {
            current_getup_time = line.substr(6);
            current_getup_time.erase(0, current_getup_time.find_first_not_of(" \t"));
            current_getup_time.erase(current_getup_time.find_last_not_of(" \t") + 1);

            std::string temp_getup_time = current_getup_time;
            if (temp_getup_time.length() == 4 && temp_getup_time.find(':') == std::string::npos) {
                 bool all_digits = true;
                 for(char c : temp_getup_time) if (!isdigit(c)) all_digits = false;
                 if(all_digits) temp_getup_time = temp_getup_time.substr(0,2) + ":" + temp_getup_time.substr(2,2);
            }

            if (time_str_to_seconds(temp_getup_time) == 0 && temp_getup_time != "00:00") {
                std::cerr << current_file_name << ": Warning: Invalid Getup time format '" << current_getup_time << "'. Using '00:00'." << std::endl;
                current_getup_time = "00:00";
            } else {
                current_getup_time = temp_getup_time;
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
        std::regex time_record_regex(R"((\d{2}:?\d{2})~(\d{2}:?\d{2})(.+))");
        std::smatch matches;

        if (std::regex_match(line, matches, time_record_regex)) {
            if (matches.size() == 4) {
                std::string start_time_str_raw = matches[1].str();
                std::string end_time_str_raw = matches[2].str();
                std::string project_path = matches[3].str();
                project_path.erase(0, project_path.find_first_not_of(" \t"));
                project_path.erase(project_path.find_last_not_of(" \t") + 1);

                auto normalize_time = [](std::string t_str) {
                    if (t_str.length() == 4 && t_str.find(':') == std::string::npos) {
                        bool all_digits = true;
                        for(char c : t_str) if(!isdigit(c)) all_digits = false;
                        if(all_digits) return t_str.substr(0,2) + ":" + t_str.substr(2,2);
                    }
                    return t_str;
                };
                std::string start_time_str = normalize_time(start_time_str_raw);
                std::string end_time_str = normalize_time(end_time_str_raw);

                int start_seconds = time_str_to_seconds(start_time_str);
                int end_seconds = time_str_to_seconds(end_time_str);

                if (start_seconds == 0 && start_time_str != "00:00") {
                     std::cerr << current_file_name << ":" << line_num << ": Invalid start time format in line: " << line << std::endl;
                     return;
                }
                 if (end_seconds == 0 && end_time_str != "00:00") {
                     std::cerr << current_file_name << ":" << line_num << ": Invalid end time format in line: " << line << std::endl;
                     return;
                }

                int duration_seconds;
                if (end_seconds < start_seconds) { // Overnight case
                    duration_seconds = (end_seconds + 24 * 3600) - start_seconds;
                } else {
                    duration_seconds = end_seconds - start_seconds;
                }

                current_time_records_buffer.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
                _process_project_path(project_path);
            } else {
                 std::cerr << current_file_name << ":" << line_num << ": Malformed time record (regex internal error, unexpected match count): " << line << std::endl;
            }
        } else {
            std::cerr << current_file_name << ":" << line_num << ": Malformed time record line (no regex match): " << line << std::endl;
        }
    }

    void _process_project_path(const std::string& project_path_orig) {
        if (!db) return;
        std::string project_path = project_path_orig;

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

        std::string current_full_path_segment = "";
        std::string parent_of_current_segment;

        for (size_t i = 0; i < segments.size(); ++i) {
            if (i == 0) {
                current_full_path_segment = segments[i];
                auto it = initial_top_level_parents.find(current_full_path_segment);
                if (it != initial_top_level_parents.end()) {
                    parent_of_current_segment = it->second;
                } else {
                    parent_of_current_segment = current_full_path_segment;
                    std::transform(parent_of_current_segment.begin(), parent_of_current_segment.end(), parent_of_current_segment.begin(), ::toupper);
                }
            } else {
                parent_of_current_segment = current_full_path_segment;
                current_full_path_segment += "_" + segments[i];
            }

            sqlite3_bind_text(stmt, 1, current_full_path_segment.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, parent_of_current_segment.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                if (sqlite3_errcode(db) != SQLITE_CONSTRAINT_PRIMARYKEY && sqlite3_errcode(db) != SQLITE_CONSTRAINT_UNIQUE) {
                    std::cerr << "Failed to insert parent_child (" << current_full_path_segment << ", " << parent_of_current_segment << "): " << sqlite3_errmsg(db) << std::endl;
                }
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }
    
    // This function now wraps all DB writes for a given date in a single transaction.
    void _store_previous_date_data() {
        if (current_date.empty() || !db || current_date_processed) {
            return;
        }

        // --- BEGIN TRANSACTION ---
        if (!execute_sql_parser(db, "BEGIN TRANSACTION;", "Begin transaction for " + current_date)) {
            return; // Failed to start transaction, abort.
        }

        bool success = true;

        // 1. Update the 'days' table with final status, remark, etc.
        sqlite3_stmt* day_stmt;
        const char* update_day_sql = "UPDATE days SET status = ?, remark = ?, getup_time = ? WHERE date = ?;";
        if (sqlite3_prepare_v2(db, update_day_sql, -1, &day_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(day_stmt, 1, current_status.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(day_stmt, 2, current_remark.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(day_stmt, 3, current_getup_time.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(day_stmt, 4, current_date.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(day_stmt) != SQLITE_DONE) {
                std::cerr << "Error updating day record for " << current_date << ": " << sqlite3_errmsg(db) << std::endl;
                success = false;
            }
            sqlite3_finalize(day_stmt);
        } else {
            std::cerr << "Failed to prepare statement for updating day record: " << sqlite3_errmsg(db) << std::endl;
            success = false;
        }

        // 2. Insert all time records, only if the day update was successful
        if (success) {
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
                         std::cerr << "Error inserting/replacing time record for " << record.date << " " << record.start << ": " << sqlite3_errmsg(db) << std::endl;
                         success = false;
                         break; // Stop processing further records for this date on failure
                    }
                    sqlite3_reset(record_stmt);
                }
                sqlite3_finalize(record_stmt);
            } else {
                std::cerr << "Failed to prepare statement for inserting time records: " << sqlite3_errmsg(db) << std::endl;
                success = false;
            }
        }
        
        // --- COMMIT OR ROLLBACK ---
        if (success) {
            execute_sql_parser(db, "COMMIT;", "Commit transaction for " + current_date);
            current_date_processed = true; // Mark as processed only on successful commit
        } else {
            execute_sql_parser(db, "ROLLBACK;", "Rollback transaction for " + current_date);
        }

        // Clear buffer regardless of outcome, to prevent reprocessing
        current_time_records_buffer.clear();
    }
};

#endif // DATA_PARSER_H
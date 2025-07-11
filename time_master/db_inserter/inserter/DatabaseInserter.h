#ifndef DATABASE_INSERTER_H
#define DATABASE_INSERTER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <sqlite3.h>
#include "time_sheet_model.h" 

/**
 * @class DatabaseInserter
 * @brief Handles all database writing operations, completely independent of the data source.
 */
class DatabaseInserter {
public:
    /**
     * @brief Constructs the inserter and opens the database connection.
     * @param db_path Path to the SQLite database file.
     */
    DatabaseInserter(const std::string& db_path);

    /**
     * @brief Destructor that finalizes statements and closes the database connection.
     */
    ~DatabaseInserter();

    /**
     * @brief Checks if the database connection is valid.
     * @return True if the database is open, false otherwise.
     */
    bool is_db_open() const;

    /**
     * @brief MODIFIED: Imports data from provided data collections into the database.
     * @details This function is now decoupled from any specific parser and accepts raw data.
     * @param days A vector of DayData structs to be inserted.
     * @param records A vector of TimeRecordInternal structs to be inserted.
     * @param parent_child_pairs An unordered_set of parent-child string pairs to be inserted.
     */
    void import_data(
        const std::vector<DayData>& days,
        const std::vector<TimeRecordInternal>& records,
        const std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs
    );

private:
    sqlite3* db; // Pointer to the SQLite database connection.
    
    // Pre-compiled SQL statement pointers for performance
    sqlite3_stmt* stmt_insert_day;
    sqlite3_stmt* stmt_insert_record;
    sqlite3_stmt* stmt_insert_parent_child;

    /**
     * @brief Initializes the database by creating tables if they don't exist.
     */
    void _initialize_database();

    /**
     * @brief Prepares (compiles) all necessary SQL INSERT statements.
     */
    void _prepare_statements();
    
    /**
     * @brief Finalizes and frees all pre-compiled statements.
     */
    void _finalize_statements();
};

/**
 * @brief A generic helper function to execute SQL statements that don't return results.
 * @param db Pointer to the database connection.
 * @param sql The SQL statement to execute.
 * @param context_msg A message to provide context in case of an error.
 * @return True if execution is successful, false otherwise.
 */
bool execute_sql_Inserter(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

#endif // DATABASE_INSERTER_H
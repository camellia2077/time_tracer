// db_inserter/inserter/facade/DatabaseInserter.hpp
#ifndef DATABASE_INSERTER_HPP
#define DATABASE_INSERTER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include "db_inserter/model/time_sheet_model.hpp"

#include "db_inserter/inserter/pipelines/DataInserter.hpp"
#include "db_inserter/inserter/pipelines/DbConnectionManager.hpp"
#include "db_inserter/inserter/pipelines/DbStatementManager.hpp"


/**
 * @class DatabaseInserter
 * @brief Facade class that simplifies the process of importing data into the database.
 *
 * This class orchestrates the operations of its sub-modules (connection, statements, and data insertion)
 * to provide a clean and simple interface for the client code.
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
    ~DatabaseInserter() = default;

    /**
     * @brief Checks if the database connection is valid.
     * @return True if the database is open, false otherwise.
     */
    bool is_db_open() const;

    /**
     * @brief Imports data from provided data collections into the database.
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
    std::unique_ptr<DbConnectionManager> connection_manager;
    std::unique_ptr<DbStatementManager> statement_manager;
    std::unique_ptr<DataInserter> data_inserter;
};

#endif // DATABASE_INSERTER_HPP
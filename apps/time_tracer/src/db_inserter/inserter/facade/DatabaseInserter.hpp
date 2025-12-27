// db_inserter/inserter/facade/DatabaseInserter.hpp
#ifndef DATABASE_INSERTER_HPP
#define DATABASE_INSERTER_HPP

#include <string>
#include <vector>
#include <memory>
#include "db_inserter/model/time_sheet_model.hpp"

// --- [FIX] Removed semicolons from the end of the include directives ---
#include "db_inserter/inserter/pipelines/DataInserter.hpp"
#include "db_inserter/inserter/pipelines/DbConnectionManager.hpp"
#include "db_inserter/inserter/pipelines/DbStatementManager.hpp"


/**
 * @class DatabaseInserter
 * @brief Facade class that simplifies the process of importing data into the database.
 */
class DatabaseInserter {
public:
    explicit DatabaseInserter(const std::string& db_path);
    ~DatabaseInserter() = default;

    bool is_db_open() const;

    /**
     * @brief Imports data from provided data collections into the database.
     * @param days A vector of DayData structs to be inserted.
     * @param records A vector of TimeRecordInternal structs to be inserted.
     */
    void import_data(
        const std::vector<DayData>& days,
        const std::vector<TimeRecordInternal>& records
    );

private:
    std::unique_ptr<DbConnectionManager> connection_manager_; // MODIFIED
    std::unique_ptr<DbStatementManager> statement_manager_; // MODIFIED
    std::unique_ptr<DataInserter> data_inserter_; // MODIFIED
};

#endif // DATABASE_INSERTER_HPP
// db_inserter/inserter/facade/DatabaseInserter.cpp
#include "DatabaseInserter.hpp"
#include <iostream>

DatabaseInserter::DatabaseInserter(const std::string& db_path) {
    connection_manager_ = std::make_unique<DbConnectionManager>(db_path); // MODIFIED
    if (connection_manager_->get_db()) { // MODIFIED
        statement_manager_ = std::make_unique<DbStatementManager>(connection_manager_->get_db()); // MODIFIED
        data_inserter_ = std::make_unique<DataInserter>( // MODIFIED
            connection_manager_->get_db(), // MODIFIED
            statement_manager_->get_insert_day_stmt(), // MODIFIED
            statement_manager_->get_insert_record_stmt(), // MODIFIED
            statement_manager_->get_select_project_id_stmt(), // MODIFIED
            statement_manager_->get_insert_project_stmt() // MODIFIED
        );
    }
}

bool DatabaseInserter::is_db_open() const {
    return connection_manager_ && connection_manager_->get_db(); // MODIFIED
}

void DatabaseInserter::import_data(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records)
{
    if (!is_db_open()) {
        std::cerr << "Database is not open. Cannot import data." << std::endl;
        return;
    }

    if (!connection_manager_->begin_transaction()) { // MODIFIED
        return; // Error message is printed inside
    }

    try {
        data_inserter_->insert_days(days); // MODIFIED
        data_inserter_->insert_records(records); // MODIFIED

        if (!connection_manager_->commit_transaction()) { // MODIFIED
            throw std::runtime_error("Failed to commit transaction.");
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred during data import: " << e.what() << std::endl;
        connection_manager_->rollback_transaction(); // MODIFIED
    }
}
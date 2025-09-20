// db_inserter/inserter/facade/DatabaseInserter.cpp
#include "DatabaseInserter.hpp"
#include <iostream>

DatabaseInserter::DatabaseInserter(const std::string& db_path) {
    connection_manager = std::make_unique<DbConnectionManager>(db_path);
    if (connection_manager->get_db()) {
        statement_manager = std::make_unique<DbStatementManager>(connection_manager->get_db());
        data_inserter = std::make_unique<DataInserter>(
            connection_manager->get_db(),
            statement_manager->get_insert_day_stmt(),
            statement_manager->get_insert_record_stmt(),
            statement_manager->get_select_project_id_stmt(),
            statement_manager->get_insert_project_stmt()
        );
    }
}

bool DatabaseInserter::is_db_open() const {
    return connection_manager && connection_manager->get_db();
}

void DatabaseInserter::import_data(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records)
{
    if (!is_db_open()) {
        std::cerr << "Database is not open. Cannot import data." << std::endl;
        return;
    }

    if (!connection_manager->begin_transaction()) {
        return; // Error message is printed inside
    }

    try {
        data_inserter->insert_days(days);
        data_inserter->insert_records(records);

        if (!connection_manager->commit_transaction()) {
            throw std::runtime_error("Failed to commit transaction.");
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred during data import: " << e.what() << std::endl;
        connection_manager->rollback_transaction();
    }
}
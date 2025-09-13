// db_inserter/inserter/facade/DatabaseInserter.cpp
#include "DatabaseInserter.hpp"
#include <iostream>
#include <stdexcept>

// --- DatabaseInserter Constructor ---

DatabaseInserter::DatabaseInserter(const std::string& db_path) {
    try {
        connection_manager = std::make_unique<DbConnectionManager>(db_path);
        if (!connection_manager->get_db()) {
            throw std::runtime_error("Failed to open database.");
        }
        statement_manager = std::make_unique<DbStatementManager>(connection_manager->get_db());
        data_inserter = std::make_unique<DataInserter>(
            connection_manager->get_db(),
            statement_manager->get_insert_day_stmt(),
            statement_manager->get_insert_record_stmt(),
            statement_manager->get_insert_parent_child_stmt()
        );
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        // Clean up on failure
        data_inserter.reset();
        statement_manager.reset();
        connection_manager.reset();
    }
}

// --- Public Member Functions ---

bool DatabaseInserter::is_db_open() const {
    return connection_manager && connection_manager->get_db();
}

void DatabaseInserter::import_data(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records,
    const std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs) {
    if (!is_db_open()) return;

    if (!connection_manager->begin_transaction()) {
        std::cerr << "Failed to begin transaction." << std::endl;
        return;
    }

    try {
        data_inserter->insert_days(days);
        data_inserter->insert_records(records);
        data_inserter->insert_parent_child_pairs(parent_child_pairs);

        if (!connection_manager->commit_transaction()) {
            std::cerr << "Failed to commit transaction." << std::endl;
            connection_manager->rollback_transaction();
        }
    } catch (const std::exception& e) {
        std::cerr << "Data insertion failed: " << e.what() << std::endl;
        connection_manager->rollback_transaction();
    }
}
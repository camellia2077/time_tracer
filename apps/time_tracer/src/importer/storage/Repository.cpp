// importer/storage/repository.cpp
#include "repository.hpp"
#include <iostream>

Repository::Repository(const std::string& db_path) {
    connection_manager_ = std::make_unique<Connection>(db_path); 
    
    if (connection_manager_->get_db()) { 

        statement_manager_ = std::make_unique<Statement>(connection_manager_->get_db()); 

        data_inserter_ = std::make_unique<Writer>( 
            connection_manager_->get_db(), 
            statement_manager_->get_insert_day_stmt(), 
            statement_manager_->get_insert_record_stmt(), 
            statement_manager_->get_select_project_id_stmt(), 
            statement_manager_->get_insert_project_stmt() 
        );
    }
}

bool Repository::is_db_open() const {
    return connection_manager_ && connection_manager_->get_db(); 
}

void Repository::import_data(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records)
{
    if (!is_db_open()) {
        std::cerr << "Database is not open. Cannot import data." << std::endl;
        return;
    }

    if (!connection_manager_->begin_transaction()) { 
        return; 
    }

    try {
        data_inserter_->insert_days(days); 
        data_inserter_->insert_records(records); 

        if (!connection_manager_->commit_transaction()) { 
            throw std::runtime_error("Failed to commit transaction.");
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred during data import: " << e.what() << std::endl;
        connection_manager_->rollback_transaction(); 
    }
}
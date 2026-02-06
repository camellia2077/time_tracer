// importer/storage/repository.cpp
#include "importer/storage/repository.hpp"

#include <iostream>

Repository::Repository(const std::string& db_path) {
  connection_manager_ = std::make_unique<Connection>(db_path);

  if (connection_manager_->get_db() != nullptr) {
    statement_manager_ =
        std::make_unique<Statement>(connection_manager_->get_db());

    data_inserter_ =
        std::make_unique<Writer>(connection_manager_->get_db(),
                                 statement_manager_->GetInsertDayStmt(),
                                 statement_manager_->GetInsertRecordStmt(),
                                 statement_manager_->GetInsertProjectStmt());
  }
}

auto Repository::IsDbOpen() const -> bool {
  return connection_manager_ && (connection_manager_->get_db() != nullptr);
}

void Repository::ImportData(const std::vector<DayData>& days,
                             const std::vector<TimeRecordInternal>& records) {
  if (!IsDbOpen()) {
    std::cerr << "Database is not open. Cannot import data." << std::endl;
    return;
  }

  if (!connection_manager_->begin_transaction()) {
    return;
  }

  try {
    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->commit_transaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception& e) {
    std::cerr << "An error occurred during data import: " << e.what()
              << std::endl;
    connection_manager_->rollback_transaction();
  }
}
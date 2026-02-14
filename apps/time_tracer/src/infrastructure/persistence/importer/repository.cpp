// infrastructure/persistence/importer/repository.cpp
#include "infrastructure/persistence/importer/repository.hpp"

#include <stdexcept>

namespace infrastructure::persistence::importer {

Repository::Repository(const std::string& db_path) {
  connection_manager_ = std::make_unique<sqlite::Connection>(db_path);

  if (connection_manager_->GetDb() != nullptr) {
    statement_manager_ =
        std::make_unique<sqlite::Statement>(connection_manager_->GetDb());

    data_inserter_ = std::make_unique<sqlite::Writer>(
        connection_manager_->GetDb(), statement_manager_->GetInsertDayStmt(),
        statement_manager_->GetInsertRecordStmt(),
        statement_manager_->GetInsertProjectStmt());
  }
}

auto Repository::IsDbOpen() const -> bool {
  return connection_manager_ && (connection_manager_->GetDb() != nullptr);
}

auto Repository::ImportData(const std::vector<DayData>& days,
                            const std::vector<TimeRecordInternal>& records)
    -> void {
  if (!IsDbOpen()) {
    throw std::runtime_error("Database is not open. Cannot import data.");
  }

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin transaction.");
  }

  try {
    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

}  // namespace infrastructure::persistence::importer

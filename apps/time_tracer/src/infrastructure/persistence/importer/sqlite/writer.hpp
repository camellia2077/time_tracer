// infrastructure/persistence/importer/sqlite/writer.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_WRITER_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_WRITER_H_

#include <sqlite3.h>

#include <memory>
#include <string>
#include <vector>

#include "application/importer/model/import_models.hpp"
#include "infrastructure/persistence/importer/sqlite/project_resolver.hpp"

namespace infrastructure::persistence::importer::sqlite {

class Writer {
 public:
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  explicit Writer(sqlite3* sqlite_db, sqlite3_stmt* stmt_day,
                  sqlite3_stmt* stmt_record, sqlite3_stmt* stmt_insert_project);
  // NOLINTEND(bugprone-easily-swappable-parameters)

  ~Writer();

  auto InsertDays(const std::vector<DayData>& days) -> void;
  auto InsertRecords(const std::vector<TimeRecordInternal>& records) -> void;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_day_;
  sqlite3_stmt* stmt_insert_record_;
  sqlite3_stmt* stmt_insert_project_;

  std::unique_ptr<ProjectResolver> project_resolver_;
};

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_WRITER_H_

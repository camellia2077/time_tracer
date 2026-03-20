module;

#include "infrastructure/sqlite_fwd.hpp"

#include <memory>
#include <vector>

#include "application/importer/model/import_models.hpp"

export module tracer.core.infrastructure.persistence.write.importer.sqlite.writer;

import tracer.core.infrastructure.persistence.write.importer.sqlite.project_resolver;

export namespace tracer::core::infrastructure::persistence::importer::sqlite {

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

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite

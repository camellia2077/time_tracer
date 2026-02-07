// importer/storage/sqlite/statement.hpp
#ifndef IMPORTER_STORAGE_SQLITE_STATEMENT_H_
#define IMPORTER_STORAGE_SQLITE_STATEMENT_H_

#include <sqlite3.h>

class Statement {
 public:
  explicit Statement(sqlite3* sqlite_db);
  ~Statement();

  [[nodiscard]] auto GetInsertDayStmt() const -> sqlite3_stmt*;
  [[nodiscard]] auto GetInsertRecordStmt() const -> sqlite3_stmt*;

  // --- [FIX] Added missing function declarations and removed the old one ---
  [[nodiscard]] auto GetInsertProjectStmt() const -> sqlite3_stmt*;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_day_ = nullptr;
  sqlite3_stmt* stmt_insert_record_ = nullptr;

  sqlite3_stmt* stmt_insert_project_ = nullptr;

  void PrepareStatements();
  void FinalizeStatements();
};

#endif  // IMPORTER_STORAGE_SQLITE_STATEMENT_H_

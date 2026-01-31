// importer/storage/sqlite/writer.hpp
#ifndef IMPORTER_STORAGE_SQLITE_WRITER_H_
#define IMPORTER_STORAGE_SQLITE_WRITER_H_

#include <sqlite3.h>

#include <memory>
#include <string>
#include <vector>

#include "importer/model/time_sheet_data.hpp"
#include "project_resolver.hpp"

class Writer {
 public:
  explicit Writer(sqlite3* db, sqlite3_stmt* stmt_day,
                  sqlite3_stmt* stmt_record, sqlite3_stmt* stmt_insert_project);

  ~Writer();

  void insert_days(const std::vector<DayData>& days);
  void insert_records(const std::vector<TimeRecordInternal>& records);

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_day_;
  sqlite3_stmt* stmt_insert_record_;

  // 保留这些句柄，传递给 Resolver 使用
  sqlite3_stmt* stmt_insert_project_;

  // [核心修改] 所有的树、缓存、解析逻辑都封装在这里
  std::unique_ptr<ProjectResolver> project_resolver_;
};

#endif  // IMPORTER_STORAGE_SQLITE_WRITER_H_
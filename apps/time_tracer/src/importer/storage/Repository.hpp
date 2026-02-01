// importer/storage/Repository.hpp
#ifndef IMPORTER_STORAGE_REPOSITORY_H_
#define IMPORTER_STORAGE_REPOSITORY_H_

#include <memory>
#include <string>
#include <vector>

#include "importer/model/time_sheet_data.hpp"

// 引用新的组件头文件
#include "importer/storage/sqlite/connection.hpp"
#include "importer/storage/sqlite/statement.hpp"
#include "importer/storage/sqlite/writer.hpp"

/**
 * @class Repository
 * @brief Facade class that simplifies the process of importing data into the
 * database.
 */
class Repository {
 public:
  explicit Repository(const std::string& db_path);
  ~Repository() = default;

  bool is_db_open() const;

  void import_data(const std::vector<DayData>& days,
                   const std::vector<TimeRecordInternal>& records);

 private:
  std::unique_ptr<Connection> connection_manager_;
  std::unique_ptr<Statement> statement_manager_;
  std::unique_ptr<Writer> data_inserter_;
};

#endif  // IMPORTER_STORAGE_REPOSITORY_H_
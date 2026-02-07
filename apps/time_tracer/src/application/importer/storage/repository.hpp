// importer/storage/repository.hpp
#ifndef IMPORTER_STORAGE_REPOSITORY_H_
#define IMPORTER_STORAGE_REPOSITORY_H_

#include <memory>
#include <string>
#include <vector>

#include "application/importer/model/time_sheet_data.hpp"

// 引用新的组件头文件
#include "application/importer/storage/sqlite/connection.hpp"
#include "application/importer/storage/sqlite/statement.hpp"
#include "application/importer/storage/sqlite/writer.hpp"

/**
 * @class Repository
 * @brief Facade class that simplifies the process of importing data into the
 * database.
 */
class Repository {
 public:
  explicit Repository(const std::string& db_path);
  ~Repository() = default;

  [[nodiscard]] auto IsDbOpen() const -> bool;

  void ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records);

 private:
  std::unique_ptr<Connection> connection_manager_;
  std::unique_ptr<Statement> statement_manager_;
  std::unique_ptr<Writer> data_inserter_;
};

#endif  // IMPORTER_STORAGE_REPOSITORY_H_
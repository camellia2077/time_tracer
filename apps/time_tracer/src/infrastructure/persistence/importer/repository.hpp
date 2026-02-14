// infrastructure/persistence/importer/repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_

#include <memory>
#include <string>
#include <vector>

#include "application/importer/model/import_models.hpp"
#include "infrastructure/persistence/importer/sqlite/connection.hpp"
#include "infrastructure/persistence/importer/sqlite/statement.hpp"
#include "infrastructure/persistence/importer/sqlite/writer.hpp"

namespace infrastructure::persistence::importer {

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

  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records) -> void;

 private:
  std::unique_ptr<sqlite::Connection> connection_manager_;
  std::unique_ptr<sqlite::Statement> statement_manager_;
  std::unique_ptr<sqlite::Writer> data_inserter_;
};

}  // namespace infrastructure::persistence::importer

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_

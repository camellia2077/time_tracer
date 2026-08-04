// infra/persistence/importer/repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_

#include <memory>
#include <string>
#include <vector>

#include "application/dto/pipeline_responses.hpp"
#include "application/pipeline/importer/model/import_models.hpp"

namespace tracer::core::infrastructure::persistence::importer::sqlite {

class Connection;
class Statement;
class Writer;

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite

namespace tracer::core::infrastructure::persistence::importer {
class Repository {
 public:
  explicit Repository(std::string db_path);
  ~Repository();

  [[nodiscard]] auto IsDbOpen() const -> bool;

  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records) -> void;
  auto ReplaceAllData(const std::vector<DayData>& days,
                      const std::vector<TimeRecordInternal>& records) -> void;
  auto ReplaceMonthData(int year, int month, const std::vector<DayData>& days,
                        const std::vector<TimeRecordInternal>& records) -> void;
  auto UpsertIngestSyncStatus(
      const tracer_core::core::dto::IngestSyncStatusEntry& entry) -> void;
  auto ReplaceIngestSyncStatuses(
      const std::vector<tracer_core::core::dto::IngestSyncStatusEntry>& entries)
      -> void;
  auto ClearIngestSyncStatus() -> void;

 private:
  auto EnsureWriteRepositoryReady() -> void;

  std::string db_path_;
  std::unique_ptr<sqlite::Connection> connection_manager_;
  std::unique_ptr<sqlite::Statement> statement_manager_;
  std::unique_ptr<sqlite::Writer> data_inserter_;
};

}  // namespace tracer::core::infrastructure::persistence::importer

namespace infrastructure::persistence::importer {

using tracer::core::infrastructure::persistence::importer::Repository;

}  // namespace infrastructure::persistence::importer

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_

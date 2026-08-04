// infra/persistence/sqlite_time_sheet_repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_

#include <string>

#include "application/ports/pipeline/i_time_sheet_write_repository.hpp"
#include "infra/persistence/importer/repository.hpp"

namespace tracer::core::infrastructure::persistence {
class SqliteTimeSheetRepository final
    : public tracer_core::application::ports::ITimeSheetWriteRepository {
 public:
  explicit SqliteTimeSheetRepository(const std::string& db_path);

  [[nodiscard]] auto IsDbOpen() const -> bool override;
  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records)
      -> void override;
  auto ReplaceAllData(const std::vector<DayData>& days,
                      const std::vector<TimeRecordInternal>& records)
      -> void override;
  auto ReplaceMonthData(int year, int month, const std::vector<DayData>& days,
                        const std::vector<TimeRecordInternal>& records)
      -> void override;
  auto UpsertIngestSyncStatus(
      const tracer_core::core::dto::IngestSyncStatusEntry& entry)
      -> void override;
  auto ReplaceIngestSyncStatuses(
      const std::vector<tracer_core::core::dto::IngestSyncStatusEntry>& entries)
      -> void override;
  auto ClearIngestSyncStatus() -> void override;

 private:
  importer::Repository repository_;
};

}  // namespace tracer::core::infrastructure::persistence

namespace infrastructure::persistence {

using tracer::core::infrastructure::persistence::SqliteTimeSheetRepository;

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_

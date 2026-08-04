#ifndef APPLICATION_PORTS_I_TIME_SHEET_WRITE_REPOSITORY_H_
#define APPLICATION_PORTS_I_TIME_SHEET_WRITE_REPOSITORY_H_

#include <vector>

#include "application/dto/pipeline_responses.hpp"
#include "application/pipeline/importer/model/import_models.hpp"

namespace tracer_core::application::ports {

// Write-side persistence boundary for validated ingest data. Implementations
// must remain lazy: constructing this port must not create or open SQLite.
class ITimeSheetWriteRepository {
 public:
  virtual ~ITimeSheetWriteRepository() = default;

  [[nodiscard]] virtual auto IsDbOpen() const -> bool = 0;
  virtual auto ImportData(const std::vector<DayData>& days,
                          const std::vector<TimeRecordInternal>& records)
      -> void = 0;
  virtual auto ReplaceAllData(const std::vector<DayData>& days,
                              const std::vector<TimeRecordInternal>& records)
      -> void = 0;
  virtual auto ReplaceMonthData(int year, int month,
                                const std::vector<DayData>& days,
                                const std::vector<TimeRecordInternal>& records)
      -> void = 0;

  virtual auto UpsertIngestSyncStatus(
      const tracer_core::core::dto::IngestSyncStatusEntry& entry) -> void = 0;
  virtual auto ReplaceIngestSyncStatuses(
      const std::vector<tracer_core::core::dto::IngestSyncStatusEntry>& entries)
      -> void = 0;
  virtual auto ClearIngestSyncStatus() -> void = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_TIME_SHEET_WRITE_REPOSITORY_H_

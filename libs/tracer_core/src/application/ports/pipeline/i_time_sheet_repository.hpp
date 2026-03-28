// application/ports/pipeline/i_time_sheet_repository.hpp
#ifndef APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_
#define APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/dto/pipeline_responses.hpp"
#include "application/pipeline/importer/model/import_models.hpp"

namespace tracer_core::application::ports {

struct PreviousActivityTail {
  std::string date;
  std::string end_time;
};

class ITimeSheetRepository {
 public:
  virtual ~ITimeSheetRepository() = default;

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
  [[nodiscard]] virtual auto ListIngestSyncStatuses(
      const tracer_core::core::dto::IngestSyncStatusRequest& request) const
      -> tracer_core::core::dto::IngestSyncStatusOutput = 0;
  [[nodiscard]] virtual auto TryGetLatestActivityTailBeforeDate(
      std::string_view date) const -> std::optional<PreviousActivityTail> = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_TIME_SHEET_REPOSITORY_H_

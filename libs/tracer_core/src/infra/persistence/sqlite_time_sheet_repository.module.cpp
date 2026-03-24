#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "infra/persistence/sqlite_time_sheet_repository.hpp"
#include "application/ports/pipeline/i_time_sheet_repository.hpp"

import tracer.core.infrastructure.persistence.write.importer.repository;

namespace tracer::core::infrastructure::persistence {

SqliteTimeSheetRepository::SqliteTimeSheetRepository(const std::string& db_path)
    : repository_(db_path) {}

auto SqliteTimeSheetRepository::IsDbOpen() const -> bool {
  // Repository construction must not be treated as a write-side side effect.
  // The ingest database may be created only when the write path is entered
  // after all validation stages have succeeded.
  return repository_.IsDbOpen();
}

auto SqliteTimeSheetRepository::ImportData(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records) -> void {
  repository_.ImportData(days, records);
}

auto SqliteTimeSheetRepository::ReplaceAllData(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records) -> void {
  repository_.ReplaceAllData(days, records);
}

auto SqliteTimeSheetRepository::ReplaceMonthData(
    int year, int month, const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records) -> void {
  repository_.ReplaceMonthData(year, month, days, records);
}

auto SqliteTimeSheetRepository::TryGetLatestActivityTailBeforeDate(
    std::string_view date) const
    -> std::optional<tracer_core::application::ports::PreviousActivityTail> {
  const auto kTail = repository_.TryGetLatestActivityTailBeforeDate(date);
  if (!kTail.has_value()) {
    return std::nullopt;
  }
  return tracer_core::application::ports::PreviousActivityTail{
      .date = kTail->date, .end_time = kTail->end_time};
}

}  // namespace tracer::core::infrastructure::persistence

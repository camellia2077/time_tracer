// infrastructure/persistence/sqlite_time_sheet_repository.cpp
#include "infrastructure/persistence/sqlite_time_sheet_repository.hpp"

namespace infrastructure::persistence {

SqliteTimeSheetRepository::SqliteTimeSheetRepository(const std::string& db_path)
    : repository_(db_path) {}

auto SqliteTimeSheetRepository::IsDbOpen() const -> bool {
  return repository_.IsDbOpen();
}

auto SqliteTimeSheetRepository::ImportData(
    const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records) -> void {
  repository_.ImportData(days, records);
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

}  // namespace infrastructure::persistence

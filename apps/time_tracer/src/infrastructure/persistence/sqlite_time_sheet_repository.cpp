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
    -> std::optional<time_tracer::application::ports::PreviousActivityTail> {
  const auto tail = repository_.TryGetLatestActivityTailBeforeDate(date);
  if (!tail.has_value()) {
    return std::nullopt;
  }
  return time_tracer::application::ports::PreviousActivityTail{
      .date = tail->date, .end_time = tail->end_time};
}

}  // namespace infrastructure::persistence

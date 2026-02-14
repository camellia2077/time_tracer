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

}  // namespace infrastructure::persistence

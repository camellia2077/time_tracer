#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "application/importer/import_service.hpp"
#include "application/ports/i_time_sheet_repository.hpp"
#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/test_support.hpp"

namespace time_tracer::application::tests {
namespace {

class FakeTimeSheetRepository final
    : public time_tracer::application::ports::ITimeSheetRepository {
 public:
  bool db_open = true;
  bool fail_import = false;
  bool fail_replace = false;

  int import_call_count = 0;
  int replace_call_count = 0;
  int replace_year = 0;
  int replace_month = 0;
  size_t import_days = 0;
  size_t import_records = 0;
  size_t replace_days = 0;
  size_t replace_records = 0;

  [[nodiscard]] auto IsDbOpen() const -> bool override { return db_open; }

  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records)
      -> void override {
    ++import_call_count;
    import_days = days.size();
    import_records = records.size();
    if (fail_import) {
      throw std::runtime_error("fake import failed");
    }
  }

  auto ReplaceMonthData(const int year, const int month,
                        const std::vector<DayData>& days,
                        const std::vector<TimeRecordInternal>& records)
      -> void override {
    ++replace_call_count;
    replace_year = year;
    replace_month = month;
    replace_days = days.size();
    replace_records = records.size();
    if (fail_replace) {
      throw std::runtime_error("fake replace failed");
    }
  }

  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(
      std::string_view /*date*/) const
      -> std::optional<
          time_tracer::application::ports::PreviousActivityTail> override {
    return std::nullopt;
  }
};

auto BuildSingleDayMap() -> std::map<std::string, std::vector<DailyLog>> {
  DailyLog day;
  day.date = "2026-02-01";
  day.getupTime = "07:00";
  day.processedActivities.push_back(BaseActivityRecord{
      .logical_id = 1,
      .start_timestamp = 25200,
      .end_timestamp = 28800,
      .start_time_str = "07:00",
      .end_time_str = "08:00",
      .project_path = "study_cpp",
      .duration_seconds = 3600,
      .remark = std::nullopt,
  });

  return {{"2026-02", {day}}};
}

auto TestReplaceMonthUsesReplacePath(TestState& state) -> void {
  FakeTimeSheetRepository repository;
  ImportService service(repository);

  const ImportStats stats = service.ImportFromMemory(
      BuildSingleDayMap(), ReplaceMonthTarget{.year = 2026, .month = 2});

  Expect(state, repository.replace_call_count == 1,
         "Replace-month import should call ReplaceMonthData once.");
  Expect(state, repository.import_call_count == 0,
         "Replace-month import should not call ImportData.");
  Expect(state,
         repository.replace_year == 2026 && repository.replace_month == 2,
         "Replace-month import should forward target year/month.");
  Expect(state, repository.replace_days == 1 && repository.replace_records == 1,
         "Replace-month import should pass parsed days/records.");
  Expect(state, stats.db_open_success && stats.transaction_success,
         "Replace-month import should report successful DB transaction.");
  Expect(state,
         stats.replaced_month.has_value() && *stats.replaced_month == "2026-02",
         "Replace-month import should expose replaced_month in stats.");
}

auto TestReplaceMonthStillRunsForEmptyData(TestState& state) -> void {
  FakeTimeSheetRepository repository;
  ImportService service(repository);

  const ImportStats stats = service.ImportFromMemory(
      {}, ReplaceMonthTarget{.year = 2026, .month = 2});

  Expect(state, repository.replace_call_count == 1,
         "Empty replace-month import should still clear target month.");
  Expect(state, repository.import_call_count == 0,
         "Empty replace-month import should not call ImportData.");
  Expect(state, repository.replace_days == 0 && repository.replace_records == 0,
         "Empty replace-month import should pass empty data vectors.");
  Expect(state, stats.db_open_success && stats.transaction_success,
         "Empty replace-month import should report successful DB transaction.");
  Expect(state,
         stats.replaced_month.has_value() && *stats.replaced_month == "2026-02",
         "Empty replace-month import should still report replaced_month.");
}

}  // namespace

auto RunImportServiceTests(TestState& state) -> void {
  TestReplaceMonthUsesReplacePath(state);
  TestReplaceMonthStillRunsForEmptyData(state);
}

}  // namespace time_tracer::application::tests

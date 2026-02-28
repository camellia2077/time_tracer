// infrastructure/tests/data_query/data_query_refactor_stats_tests.cpp
#include <sqlite3.h>

#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/list_query_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"
#include "infrastructure/query/data/stats/day_duration_stats_calculator.hpp"
#include "infrastructure/query/data/stats/report_chart_stats_calculator.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"
#include "infrastructure/tests/data_query/data_query_refactor_test_internal.hpp"

namespace android_runtime_tests::data_query_refactor_internal {
namespace {

using nlohmann::json;

constexpr double kDoubleTolerance = 1e-9;
constexpr long long kDuration3600 = 3600;
constexpr long long kDuration1800 = 1800;
constexpr double kDuration1800Double = 1800.0;
constexpr double kVariance2160000Double = 2160000.0;
constexpr long long kDuration5400 = 5400;

auto BuildStatsSampleRows()
    -> std::vector<tracer_core::infrastructure::query::data::DayDurationRow> {
  return {
      {.date = "2026-02-01", .total_seconds = kDuration3600},
      {.date = "2026-02-02", .total_seconds = kDuration1800},
      {.date = "2026-02-03", .total_seconds = 0},
  };
}

auto BuildSparseReportChartRows()
    -> std::vector<tracer_core::infrastructure::query::data::DayDurationRow> {
  return {
      {.date = "2026-02-01", .total_seconds = kDuration3600},
      {.date = "2026-02-03", .total_seconds = kDuration1800},
  };
}

auto OpenSeededDatabaseOrRecordFailure(int& failures) -> ScopedSqlite {
  auto database = OpenInMemoryDatabase();
  Expect(database != nullptr,
         "orchestrator snapshot should open in-memory sqlite database.",
         failures);
  if (database == nullptr) {
    return {nullptr};
  }

  const bool kSeeded = SeedDataQueryFixture(database.get());
  Expect(kSeeded, "orchestrator snapshot should seed sqlite fixture.",
         failures);
  if (!kSeeded) {
    return {nullptr};
  }

  return database;
}

auto TestDayDurationStatsCalculator(int& failures) -> void {
  const auto kRows = BuildStatsSampleRows();
  const auto kStats =
      tracer_core::infrastructure::query::data::stats::ComputeDayDurationStats(
          kRows);

  Expect(kStats.count == 3, "stats calculator should keep sample count.",
         failures);
  ExpectNear(kStats.mean_seconds, kDuration1800Double,
             "stats calculator mean should match expected value.", failures);
  ExpectNear(kStats.variance_seconds, kVariance2160000Double,
             "stats calculator variance should match expected value.",
             failures);
  ExpectNear(kStats.mad_seconds, kDuration1800Double,
             "stats calculator MAD should match expected value.", failures);
}

auto TestReportChartSeriesCalculator(int& failures) -> void {
  const auto kSparseRows = BuildSparseReportChartRows();
  const auto kResult =
      tracer_core::infrastructure::query::data::stats::BuildReportChartSeries(
          "2026-02-01", "2026-02-03", kSparseRows);

  Expect(kResult.series.size() == 3,
         "report chart series should fill missing dates with zero.", failures);
  if (kResult.series.size() == 3) {
    Expect(kResult.series[0].duration_seconds == kDuration3600,
           "report chart first day should keep known duration.", failures);
    Expect(kResult.series[1].duration_seconds == 0,
           "report chart middle day should be zero-filled.", failures);
    Expect(kResult.series[2].duration_seconds == kDuration1800,
           "report chart last day should keep known duration.", failures);
    Expect(kResult.series[1].epoch_day == kResult.series[0].epoch_day + 1,
           "report chart epoch_day should increment daily.", failures);
    Expect(kResult.series[2].epoch_day == kResult.series[1].epoch_day + 1,
           "report chart epoch_day should remain contiguous.", failures);
  }
  Expect(kResult.stats.total_duration_seconds == kDuration5400,
         "report chart total duration should match sum.", failures);
  Expect(kResult.stats.active_days == 2,
         "report chart active days should count non-zero days.", failures);
  Expect(kResult.stats.range_days == 3,
         "report chart range_days should cover inclusive interval.", failures);
  Expect(kResult.stats.average_duration_seconds == kDuration1800,
         "report chart average should use range_days denominator.", failures);
}

auto TestSemanticDayStatsSnapshot(int& failures) -> void {
  using tracer_core::infrastructure::query::data::stats::
      ComputeDayDurationStats;

  const auto kRows = BuildStatsSampleRows();
  const auto kStats = ComputeDayDurationStats(kRows);
  const std::string kSemantic = tracer_core::infrastructure::query::data::
      renderers::RenderDayDurationStatsOutput(kRows, kStats, 2, true);

  const auto kPayload = json::parse(kSemantic);
  Expect(kPayload.value("schema_version", 0) == 1,
         "days-stats semantic snapshot should keep schema_version=1.",
         failures);
  Expect(kPayload.value("action", std::string{}) == "days_stats",
         "days-stats semantic snapshot should keep action field.", failures);
  Expect(kPayload.value("output_mode", std::string{}) == "semantic_json",
         "days-stats semantic snapshot should keep output_mode.", failures);

  const auto kStatsIt = kPayload.find("stats");
  Expect(kStatsIt != kPayload.end() && kStatsIt->is_object(),
         "days-stats semantic snapshot should include `stats` object.",
         failures);
  if (kStatsIt == kPayload.end() || !kStatsIt->is_object()) {
    return;
  }

  static constexpr std::array<std::string_view, 13> kStatsKeys = {
      "count",          "mean_seconds", "variance_seconds", "stddev_seconds",
      "median_seconds", "p25_seconds",  "p75_seconds",      "p90_seconds",
      "p95_seconds",    "min_seconds",  "max_seconds",      "iqr_seconds",
      "mad_seconds",
  };
  for (const auto kKey : kStatsKeys) {
    Expect(kStatsIt->contains(kKey),
           std::string("days-stats semantic snapshot missing field: ") +
               std::string(kKey),
           failures);
  }

  Expect(kPayload.value("total_count", -1) == 3,
         "days-stats semantic snapshot should keep total_count.", failures);
  Expect(kPayload.value("top_n_requested", -1) == 2,
         "days-stats semantic snapshot should keep top_n_requested.", failures);
}

auto ExpectContiguousEpochDaySeries(const json& series_payload,
                                    const std::string& epoch_day_message,
                                    const std::string& contiguous_message,
                                    int& failures) -> void {
  std::optional<long long> previous_epoch_day;
  for (const auto& row : series_payload) {
    const bool kHasEpochDay = row.is_object() && row.contains("epoch_day") &&
                              row["epoch_day"].is_number_integer();
    Expect(kHasEpochDay, epoch_day_message, failures);
    if (!kHasEpochDay) {
      continue;
    }
    const long long kCurrentEpochDay = row["epoch_day"].get<long long>();
    if (previous_epoch_day.has_value()) {
      Expect(kCurrentEpochDay == *previous_epoch_day + 1, contiguous_message,
             failures);
    }
    previous_epoch_day = kCurrentEpochDay;
  }
}

auto CheckYearsOrchestratorSemanticSnapshot(sqlite3* database, int& failures)
    -> bool {
  const auto kYearsOutput =
      tracer_core::infrastructure::query::data::orchestrators::HandleYearsQuery(
          database, true);
  Expect(kYearsOutput.ok, "years orchestrator should succeed.", failures);
  if (!kYearsOutput.ok) {
    return false;
  }
  const auto kYearsPayload = json::parse(kYearsOutput.content);
  Expect(kYearsPayload.value("action", std::string{}) == "years",
         "years orchestrator semantic snapshot should keep action=years.",
         failures);
  Expect(kYearsPayload.value("output_mode", std::string{}) == "semantic_json",
         "years orchestrator semantic snapshot should keep semantic output.",
         failures);
  const auto kItemsIt = kYearsPayload.find("items");
  const bool kHasItemsArray =
      kItemsIt != kYearsPayload.end() && kItemsIt->is_array();
  Expect(kHasItemsArray,
         "years orchestrator semantic snapshot should include items array.",
         failures);
  if (kHasItemsArray) {
    Expect(kItemsIt->size() == 1 && (*kItemsIt)[0] == "2026",
           "years orchestrator semantic snapshot should return seeded year.",
           failures);
  }
  return true;
}

auto CheckDaysStatsOrchestratorSemanticSnapshot(sqlite3* database,
                                                int& failures) -> bool {
  using tracer_core::core::dto::DataQueryAction;
  using tracer_core::core::dto::DataQueryRequest;
  using tracer_core::infrastructure::query::data::QueryFilters;

  DataQueryRequest stats_request;
  stats_request.action = DataQueryAction::kDaysStats;
  stats_request.top_n = 1;
  QueryFilters base_filters;
  const auto kStatsOutput =
      tracer_core::infrastructure::query::data::orchestrators::
          HandleDaysStatsQuery(database, stats_request, base_filters, true);
  Expect(kStatsOutput.ok, "days-stats orchestrator should succeed.", failures);
  if (!kStatsOutput.ok) {
    return false;
  }
  const auto kStatsPayload = json::parse(kStatsOutput.content);
  Expect(kStatsPayload.value("action", std::string{}) == "days_stats",
         "days-stats orchestrator semantic snapshot should keep action.",
         failures);
  const auto kStatsObjectIt = kStatsPayload.find("stats");
  const bool kHasStatsObject =
      kStatsObjectIt != kStatsPayload.end() && kStatsObjectIt->is_object();
  Expect(kHasStatsObject,
         "days-stats orchestrator semantic snapshot should include stats.",
         failures);
  if (kHasStatsObject) {
    Expect(kStatsObjectIt->value("count", -1) == 2,
           "days-stats orchestrator semantic snapshot should count "
           "non-empty rows.",
           failures);
    Expect(kStatsObjectIt->value("mean_seconds", -1.0) == 2700.0,
           "days-stats orchestrator semantic snapshot should keep mean "
           "contract.",
           failures);
  }
  return true;
}

auto CheckReportChartOrchestratorSemanticSnapshot(sqlite3* database,
                                                  int& failures) -> bool {
  using tracer_core::core::dto::DataQueryAction;
  using tracer_core::core::dto::DataQueryRequest;
  constexpr long long kExpectedAverageDurationSeconds = 1800LL;

  DataQueryRequest report_chart_request;
  report_chart_request.action = DataQueryAction::kReportChart;
  report_chart_request.from_date = "2026-02-01";
  report_chart_request.to_date = "2026-02-03";
  report_chart_request.root = "study";
  const auto kChartOutput =
      tracer_core::infrastructure::query::data::orchestrators::
          HandleReportChartQuery(database, report_chart_request, true);
  Expect(kChartOutput.ok, "report-chart orchestrator should succeed.",
         failures);
  if (!kChartOutput.ok) {
    return false;
  }
  const auto kChartPayload = json::parse(kChartOutput.content);
  Expect(kChartPayload.value("action", std::string{}) == "report_chart",
         "report-chart orchestrator semantic snapshot should keep action.",
         failures);
  Expect(kChartPayload.value("selected_root", std::string{}) == "study",
         "report-chart orchestrator semantic snapshot should keep selected "
         "root.",
         failures);
  Expect(kChartPayload.value("total_duration_seconds", -1LL) == 5400LL,
         "report-chart orchestrator semantic snapshot should keep total "
         "duration.",
         failures);
  Expect(kChartPayload.value("average_duration_seconds", -1LL) ==
             kExpectedAverageDurationSeconds,
         "report-chart orchestrator semantic snapshot should keep average.",
         failures);
  Expect(kChartPayload.value("range_days", -1) == 3,
         "report-chart orchestrator semantic snapshot should keep range_days.",
         failures);
  const auto kChartSeriesIt = kChartPayload.find("series");
  const bool kHasChartSeries =
      kChartSeriesIt != kChartPayload.end() && kChartSeriesIt->is_array();
  Expect(kHasChartSeries,
         "report-chart orchestrator semantic snapshot should include series "
         "array.",
         failures);
  if (kHasChartSeries) {
    Expect(kChartSeriesIt->size() == 3U,
           "report-chart orchestrator semantic snapshot should include three "
           "points for requested range.",
           failures);
    ExpectContiguousEpochDaySeries(
        *kChartSeriesIt,
        "report-chart semantic snapshot series rows should include integer "
        "epoch_day.",
        "report-chart semantic snapshot epoch_day should be contiguous.",
        failures);
  }

  DataQueryRequest missing_root_request = report_chart_request;
  missing_root_request.root = "nosuchroot";
  const auto kMissingRootOutput =
      tracer_core::infrastructure::query::data::orchestrators::
          HandleReportChartQuery(database, missing_root_request, true);
  Expect(kMissingRootOutput.ok,
         "report-chart missing-root fallback should succeed.", failures);
  if (!kMissingRootOutput.ok) {
    return false;
  }
  const auto kMissingRootPayload = json::parse(kMissingRootOutput.content);
  Expect(
      kMissingRootPayload.value("selected_root", std::string{}) == "nosuchroot",
      "report-chart missing-root fallback should echo selected_root.",
      failures);
  Expect(kMissingRootPayload.value("total_duration_seconds", -1LL) == 0LL,
         "report-chart missing-root fallback should return zero total.",
         failures);
  Expect(kMissingRootPayload.value("active_days", -1) == 0,
         "report-chart missing-root fallback should return zero active days.",
         failures);
  const auto kMissingRootSeriesIt = kMissingRootPayload.find("series");
  const bool kHasMissingRootSeries =
      kMissingRootSeriesIt != kMissingRootPayload.end() &&
      kMissingRootSeriesIt->is_array();
  Expect(kHasMissingRootSeries,
         "report-chart missing-root fallback should keep series array.",
         failures);
  if (kHasMissingRootSeries) {
    ExpectContiguousEpochDaySeries(
        *kMissingRootSeriesIt,
        "report-chart missing-root fallback should keep epoch_day for series "
        "rows.",
        "report-chart missing-root fallback epoch_day should be contiguous.",
        failures);
  }
  return true;
}

auto TestOrchestratorRendererSemanticSnapshot(int& failures) -> void {
  const auto kDatabase = OpenSeededDatabaseOrRecordFailure(failures);
  if (kDatabase == nullptr) {
    return;
  }

  if (!CheckYearsOrchestratorSemanticSnapshot(kDatabase.get(), failures)) {
    return;
  }
  if (!CheckDaysStatsOrchestratorSemanticSnapshot(kDatabase.get(), failures)) {
    return;
  }
  if (!CheckReportChartOrchestratorSemanticSnapshot(kDatabase.get(),
                                                    failures)) {
    return;
  }
}

}  // namespace

void SqliteCloser::operator()(sqlite3* database) const {
  if (database != nullptr) {
    sqlite3_close(database);
  }
}

auto OpenInMemoryDatabase() -> ScopedSqlite {
  sqlite3* database = nullptr;
  if (sqlite3_open(":memory:", &database) != SQLITE_OK || database == nullptr) {
    if (database != nullptr) {
      sqlite3_close(database);
    }
    return {nullptr};
  }
  return ScopedSqlite(database);
}

auto SeedDataQueryFixture(sqlite3* database) -> bool {
  constexpr std::string_view kCreateDaysTable =
      "CREATE TABLE days ("
      "  date TEXT PRIMARY KEY,"
      "  year INTEGER NOT NULL,"
      "  month INTEGER NOT NULL"
      ");";
  constexpr std::string_view kCreateTimeRecordsTable =
      "CREATE TABLE time_records ("
      "  date TEXT NOT NULL,"
      "  duration INTEGER NOT NULL,"
      "  project_path_snapshot TEXT,"
      "  activity_remark TEXT"
      ");";
  constexpr std::string_view kInsertDays =
      "INSERT INTO days(date, year, month) VALUES "
      "('2026-02-01', 2026, 2),"
      "('2026-02-02', 2026, 2),"
      "('2026-02-03', 2026, 2);";
  constexpr std::string_view kInsertRecords =
      "INSERT INTO time_records(date, duration, project_path_snapshot, "
      "activity_remark) VALUES "
      "('2026-02-01', 3600, 'study_cpp', ''),"
      "('2026-02-03', 1800, 'study_cpp', '');";

  return ExecuteSql(database, std::string(kCreateDaysTable)) &&
         ExecuteSql(database, std::string(kCreateTimeRecordsTable)) &&
         ExecuteSql(database, std::string(kInsertDays)) &&
         ExecuteSql(database, std::string(kInsertRecords));
}

auto ReadFileText(const std::filesystem::path& target_path)
    -> std::optional<std::string> {
  std::ifstream input(target_path, std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }
  std::string content((std::istreambuf_iterator<char>(input)),
                      std::istreambuf_iterator<char>());
  return content;
}

auto Expect(bool condition, const std::string& message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto ExpectNear(double lhs, double rhs, const std::string& message,
                int& failures) -> void {
  Expect(std::abs(lhs - rhs) < kDoubleTolerance, message, failures);
}

auto RunDataQueryRefactorStatsTests(int& failures) -> void {
  TestDayDurationStatsCalculator(failures);
  TestReportChartSeriesCalculator(failures);
  TestSemanticDayStatsSnapshot(failures);
  TestOrchestratorRendererSemanticSnapshot(failures);
}

}  // namespace android_runtime_tests::data_query_refactor_internal

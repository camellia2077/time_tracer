// infrastructure/tests/data_query/data_query_refactor_stats_scenario_tests.cpp
import tracer.core.infrastructure.query.data.orchestrators;
import tracer.core.infrastructure.query.data.renderers;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.stats;

#include <array>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "application/dto/core_requests.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"
#include "infrastructure/tests/data_query/data_query_refactor_test_internal.hpp"

namespace android_runtime_tests::data_query_refactor_internal {
namespace {

using nlohmann::json;
using tracer_core::core::dto::DataQueryOutputMode;
namespace data_query = tracer::core::infrastructure::query::data;
namespace data_query_orchestrators = data_query::orchestrators;
namespace data_query_renderers = data_query::renderers;
namespace data_query_stats =
    tracer::core::infrastructure::query::data::stats;
using data_query::DayDurationRow;
using data_query::QueryFilters;

constexpr long long kDuration3600 = 3600;
constexpr long long kDuration1800 = 1800;
constexpr double kDuration1800Double = 1800.0;
constexpr double kVariance2160000Double = 2160000.0;
constexpr long long kDuration5400 = 5400;

auto BuildStatsSampleRows()
    -> std::vector<DayDurationRow> {
  return {
      {.date = "2026-02-01", .total_seconds = kDuration3600},
      {.date = "2026-02-02", .total_seconds = kDuration1800},
      {.date = "2026-02-03", .total_seconds = 0},
  };
}

auto BuildSparseReportChartRows()
    -> std::vector<DayDurationRow> {
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
  const auto kStats = data_query_stats::ComputeDayDurationStats(kRows);

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
  const auto kResult = data_query_stats::BuildReportChartSeries(
      {.start_date = "2026-02-01", .end_date = "2026-02-03"}, kSparseRows);

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
  using data_query_stats::ComputeDayDurationStats;

  const auto kRows = BuildStatsSampleRows();
  const auto kStats = ComputeDayDurationStats(kRows);
  const std::string kSemantic = data_query_renderers::RenderDayDurationStatsOutput(
      kRows, kStats, 2, DataQueryOutputMode::kSemanticJson);

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
  const auto kYearsOutput = data_query_orchestrators::HandleYearsQuery(
      database, DataQueryOutputMode::kSemanticJson);
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
  constexpr double kExpectedMeanSeconds = 2700.0;

  DataQueryRequest stats_request;
  stats_request.action = DataQueryAction::kDaysStats;
  stats_request.top_n = 1;
  QueryFilters base_filters;
  const auto kStatsOutput = data_query_orchestrators::HandleDaysStatsQuery(
      database, stats_request, base_filters, DataQueryOutputMode::kSemanticJson);
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
    Expect(kStatsObjectIt->value("mean_seconds", -1.0) == kExpectedMeanSeconds,
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
  constexpr long long kExpectedTotalDurationSeconds = 5400LL;
  constexpr long long kExpectedAverageDurationSeconds = 1800LL;

  DataQueryRequest report_chart_request;
  report_chart_request.action = DataQueryAction::kReportChart;
  report_chart_request.from_date = "2026-02-01";
  report_chart_request.to_date = "2026-02-03";
  report_chart_request.root = "study";
  const auto kChartOutput = data_query_orchestrators::HandleReportChartQuery(
      database, report_chart_request, DataQueryOutputMode::kSemanticJson);
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
  Expect(kChartPayload.value("total_duration_seconds", -1LL) ==
             kExpectedTotalDurationSeconds,
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
  const auto kMissingRootOutput = data_query_orchestrators::HandleReportChartQuery(
      database, missing_root_request, DataQueryOutputMode::kSemanticJson);
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

auto TestDerivedStatusExerciseFilters(int& failures) -> void {
  const auto kDatabase = OpenInMemoryDatabase();
  Expect(kDatabase != nullptr,
         "derived status/exercise filter test should open sqlite database.",
         failures);
  if (kDatabase == nullptr) {
    return;
  }

  const bool kCreatedDays = ExecuteSql(
      kDatabase.get(),
      "CREATE TABLE days ("
      "date TEXT PRIMARY KEY,"
      "year INTEGER NOT NULL,"
      "month INTEGER NOT NULL,"
      "sleep INTEGER NOT NULL DEFAULT 0,"
      "remark TEXT NOT NULL DEFAULT '',"
      "getup_time TEXT"
      ");");
  const bool kCreatedRecords = ExecuteSql(
      kDatabase.get(),
      "CREATE TABLE time_records ("
      "date TEXT NOT NULL,"
      "duration INTEGER NOT NULL,"
      "project_path_snapshot TEXT,"
      "activity_remark TEXT"
      ");");
  const bool kSeededDays = ExecuteSql(
      kDatabase.get(),
      "INSERT INTO days(date, year, month, sleep, remark, getup_time) VALUES "
      "('2026-02-01', 2026, 2, 0, '', '07:00'),"
      "('2026-02-02', 2026, 2, 0, '', '07:30'),"
      "('2026-02-03', 2026, 2, 0, '', '08:00');");
  const bool kSeededRecords = ExecuteSql(
      kDatabase.get(),
      "INSERT INTO time_records(date, duration, project_path_snapshot, activity_remark) VALUES "
      "('2026-02-01', 3600, 'study_cpp', ''),"
      "('2026-02-02', 1800, 'exercise_cardio', ''),"
      "('2026-02-03', 600, 'meal_short', '');");
  Expect(kCreatedDays && kCreatedRecords && kSeededDays && kSeededRecords,
         "derived status/exercise filter test should seed sqlite fixture.",
         failures);
  if (!(kCreatedDays && kCreatedRecords && kSeededDays && kSeededRecords)) {
    return;
  }

  QueryFilters status_true_filters;
  status_true_filters.status = 1;
  const auto kStatusTrueDates =
      data_query::QueryDatesByFilters(kDatabase.get(), status_true_filters);
  Expect(kStatusTrueDates.size() == 1U && kStatusTrueDates.front() == "2026-02-01",
         "status=true filter should match only study days.", failures);

  QueryFilters status_false_filters;
  status_false_filters.status = 0;
  const auto kStatusFalseDates =
      data_query::QueryDatesByFilters(kDatabase.get(), status_false_filters);
  Expect(kStatusFalseDates.size() == 2U &&
             kStatusFalseDates[0] == "2026-02-02" &&
             kStatusFalseDates[1] == "2026-02-03",
         "status=false filter should exclude study days.", failures);

  QueryFilters exercise_true_filters;
  exercise_true_filters.exercise = 1;
  const auto kExerciseTrueDates =
      data_query::QueryDatesByFilters(kDatabase.get(), exercise_true_filters);
  Expect(kExerciseTrueDates.size() == 1U &&
             kExerciseTrueDates.front() == "2026-02-02",
         "exercise=true filter should match only exercise days.", failures);

  QueryFilters exercise_false_filters;
  exercise_false_filters.exercise = 0;
  const auto kExerciseFalseDates =
      data_query::QueryDatesByFilters(kDatabase.get(), exercise_false_filters);
  Expect(kExerciseFalseDates.size() == 2U &&
             kExerciseFalseDates[0] == "2026-02-01" &&
             kExerciseFalseDates[1] == "2026-02-03",
         "exercise=false filter should exclude exercise days.", failures);
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

auto RunDataQueryRefactorStatsScenarioTests(int& failures) -> void {
  TestDayDurationStatsCalculator(failures);
  TestReportChartSeriesCalculator(failures);
  TestSemanticDayStatsSnapshot(failures);
  TestDerivedStatusExerciseFilters(failures);
  TestOrchestratorRendererSemanticSnapshot(failures);
}

}  // namespace android_runtime_tests::data_query_refactor_internal

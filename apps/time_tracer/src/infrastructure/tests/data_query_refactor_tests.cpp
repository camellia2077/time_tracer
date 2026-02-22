#include <sqlite3.h>

#include <array>
#include <cmath>
#include <exception>
#include <filesystem>
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
#include "infrastructure/query/data/orchestrators/date_range_resolver.hpp"
#include "infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/list_query_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"
#include "infrastructure/query/data/stats/day_duration_stats_calculator.hpp"
#include "infrastructure/query/data/stats/report_chart_stats_calculator.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

using nlohmann::json;

namespace android_runtime_tests {
namespace {

constexpr double kDoubleTolerance = 1e-9;

struct SqliteCloser {
  void operator()(sqlite3* database) const {
    if (database != nullptr) {
      sqlite3_close(database);
    }
  }
};

using ScopedSqlite = std::unique_ptr<sqlite3, SqliteCloser>;

auto OpenInMemoryDatabase() -> ScopedSqlite {
  sqlite3* database = nullptr;
  if (sqlite3_open(":memory:", &database) != SQLITE_OK || database == nullptr) {
    if (database != nullptr) {
      sqlite3_close(database);
    }
    return ScopedSqlite(nullptr);
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

auto TestDayDurationStatsCalculator(int& failures) -> void {
  using time_tracer::infrastructure::query::data::DayDurationRow;

  const std::vector<DayDurationRow> kRows = {
      {.date = "2026-02-01", .total_seconds = 3600},
      {.date = "2026-02-02", .total_seconds = 1800},
      {.date = "2026-02-03", .total_seconds = 0},
  };

  const auto kStats =
      time_tracer::infrastructure::query::data::stats::ComputeDayDurationStats(
          kRows);

  Expect(kStats.count == 3, "stats calculator should keep sample count.",
         failures);
  ExpectNear(kStats.mean_seconds, 1800.0,
             "stats calculator mean should match expected value.", failures);
  ExpectNear(kStats.variance_seconds, 2160000.0,
             "stats calculator variance should match expected value.",
             failures);
  ExpectNear(kStats.mad_seconds, 1800.0,
             "stats calculator MAD should match expected value.", failures);
}

auto TestReportChartSeriesCalculator(int& failures) -> void {
  using time_tracer::infrastructure::query::data::DayDurationRow;

  const std::vector<DayDurationRow> kSparseRows = {
      {.date = "2026-02-01", .total_seconds = 3600},
      {.date = "2026-02-03", .total_seconds = 1800},
  };
  const auto kResult =
      time_tracer::infrastructure::query::data::stats::BuildReportChartSeries(
          "2026-02-01", "2026-02-03", kSparseRows);

  Expect(kResult.series.size() == 3,
         "report chart series should fill missing dates with zero.", failures);
  if (kResult.series.size() == 3) {
    Expect(kResult.series[0].duration_seconds == 3600,
           "report chart first day should keep known duration.", failures);
    Expect(kResult.series[1].duration_seconds == 0,
           "report chart middle day should be zero-filled.", failures);
    Expect(kResult.series[2].duration_seconds == 1800,
           "report chart last day should keep known duration.", failures);
  }
  Expect(kResult.stats.total_duration_seconds == 5400,
         "report chart total duration should match sum.", failures);
  Expect(kResult.stats.active_days == 2,
         "report chart active days should count non-zero days.", failures);
  Expect(kResult.stats.range_days == 3,
         "report chart range_days should cover inclusive interval.", failures);
  Expect(kResult.stats.average_duration_seconds == 1800,
         "report chart average should use range_days denominator.", failures);
}

auto TestSemanticDayStatsSnapshot(int& failures) -> void {
  using time_tracer::infrastructure::query::data::DayDurationRow;
  using time_tracer::infrastructure::query::data::stats::
      ComputeDayDurationStats;

  const std::vector<DayDurationRow> kRows = {
      {.date = "2026-02-01", .total_seconds = 3600},
      {.date = "2026-02-02", .total_seconds = 1800},
      {.date = "2026-02-03", .total_seconds = 0},
  };
  const auto kStats = ComputeDayDurationStats(kRows);
  const std::string kSemantic = time_tracer::infrastructure::query::data::
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
  for (const auto key : kStatsKeys) {
    Expect(kStatsIt->contains(key),
           std::string("days-stats semantic snapshot missing field: ") +
               std::string(key),
           failures);
  }

  Expect(kPayload.value("total_count", -1) == 3,
         "days-stats semantic snapshot should keep total_count.", failures);
  Expect(kPayload.value("top_n_requested", -1) == 2,
         "days-stats semantic snapshot should keep top_n_requested.", failures);
}

auto TestDateRangeResolver(int& failures) -> void {
  using time_tracer::infrastructure::query::data::orchestrators::
      ResolveExplicitDateRange;
  using time_tracer::infrastructure::query::data::orchestrators::
      ResolveRollingDateRange;

  const auto kExplicitRange = ResolveExplicitDateRange(
      "20260201", "20260203",
      "report-chart requires both --from-date and "
      "--to-date.",
      "report-chart invalid range: from_date must be <= "
      "to_date.",
      "report-chart resolved invalid date range.");
  Expect(kExplicitRange.has_value(),
         "explicit date range should be produced when both boundaries exist.",
         failures);
  if (kExplicitRange.has_value()) {
    Expect(kExplicitRange->start_date == "2026-02-01",
           "explicit range should normalize start date.", failures);
    Expect(kExplicitRange->end_date == "2026-02-03",
           "explicit range should normalize end date.", failures);
  }

  bool threw_missing_boundary = false;
  try {
    static_cast<void>(ResolveExplicitDateRange(
        "20260201", std::nullopt,
        "report-chart requires both --from-date and --to-date.",
        "report-chart invalid range: from_date must be <= to_date.",
        "report-chart resolved invalid date range."));
  } catch (const std::exception& ex) {
    threw_missing_boundary =
        Contains(ex.what(), "requires both --from-date and --to-date");
  }
  Expect(threw_missing_boundary,
         "explicit resolver should reject missing boundary parameter.",
         failures);

  const auto kRollingRange = ResolveRollingDateRange(3);
  Expect(kRollingRange.start_date.size() == 10 &&
             kRollingRange.end_date.size() == 10,
         "rolling range should return ISO yyyy-mm-dd boundaries.", failures);
}

auto TestRendererGateway(int& failures) -> void {
  const std::vector<std::string> kYears = {"2024", "2025"};
  const std::string kText =
      time_tracer::infrastructure::query::data::renderers::RenderListOutput(
          "years", kYears, false);
  Expect(Contains(kText, "Total: 2"),
         "text renderer should preserve total footer.", failures);

  const std::string kSemantic =
      time_tracer::infrastructure::query::data::renderers::RenderListOutput(
          "years", kYears, true);
  const auto kSemanticJson = json::parse(kSemantic);
  Expect(kSemanticJson.value("schema_version", 0) == 1,
         "semantic renderer should emit schema_version.", failures);
  Expect(kSemanticJson.value("action", std::string{}) == "years",
         "semantic renderer should emit action field.", failures);
  Expect(kSemanticJson.value("output_mode", std::string{}) == "semantic_json",
         "semantic renderer should emit output_mode.", failures);

  const std::string kWrappedRaw = time_tracer::infrastructure::query::data::
      renderers::RenderJsonObjectOutput("report_chart", "not-json", true);
  const auto kWrappedRawJson = json::parse(kWrappedRaw);
  Expect(kWrappedRawJson.contains("raw_content"),
         "semantic json wrapper should preserve raw payload on parse failure.",
         failures);
}

auto TestOrchestratorRendererSemanticSnapshot(int& failures) -> void {
  using time_tracer::core::dto::DataQueryAction;
  using time_tracer::core::dto::DataQueryRequest;
  using time_tracer::infrastructure::query::data::QueryFilters;

  const auto kDatabase = OpenInMemoryDatabase();
  Expect(kDatabase != nullptr,
         "orchestrator snapshot should open in-memory sqlite database.",
         failures);
  if (kDatabase == nullptr) {
    return;
  }

  const bool seeded = SeedDataQueryFixture(kDatabase.get());
  Expect(seeded, "orchestrator snapshot should seed sqlite fixture.", failures);
  if (!seeded) {
    return;
  }

  const auto kYearsOutput =
      time_tracer::infrastructure::query::data::orchestrators::HandleYearsQuery(
          kDatabase.get(), true);
  Expect(kYearsOutput.ok, "years orchestrator should succeed.", failures);
  if (!kYearsOutput.ok) {
    return;
  }
  const auto kYearsPayload = json::parse(kYearsOutput.content);
  Expect(kYearsPayload.value("action", std::string{}) == "years",
         "years orchestrator semantic snapshot should keep action=years.",
         failures);
  Expect(kYearsPayload.value("output_mode", std::string{}) == "semantic_json",
         "years orchestrator semantic snapshot should keep semantic output.",
         failures);
  const auto kItemsIt = kYearsPayload.find("items");
  Expect(kItemsIt != kYearsPayload.end() && kItemsIt->is_array(),
         "years orchestrator semantic snapshot should include items array.",
         failures);
  if (kItemsIt != kYearsPayload.end() && kItemsIt->is_array()) {
    Expect(kItemsIt->size() == 1 && (*kItemsIt)[0] == "2026",
           "years orchestrator semantic snapshot should return seeded year.",
           failures);
  }

  DataQueryRequest stats_request;
  stats_request.action = DataQueryAction::kDaysStats;
  stats_request.top_n = 1;
  QueryFilters base_filters;
  const auto kStatsOutput = time_tracer::infrastructure::query::data::
      orchestrators::HandleDaysStatsQuery(kDatabase.get(), stats_request,
                                          base_filters, true);
  Expect(kStatsOutput.ok, "days-stats orchestrator should succeed.", failures);
  if (!kStatsOutput.ok) {
    return;
  }
  const auto kStatsPayload = json::parse(kStatsOutput.content);
  Expect(kStatsPayload.value("action", std::string{}) == "days_stats",
         "days-stats orchestrator semantic snapshot should keep action.",
         failures);
  const auto kStatsObject = kStatsPayload.find("stats");
  Expect(kStatsObject != kStatsPayload.end() && kStatsObject->is_object(),
         "days-stats orchestrator semantic snapshot should include stats.",
         failures);
  if (kStatsObject != kStatsPayload.end() && kStatsObject->is_object()) {
    Expect(kStatsObject->value("count", -1) == 2,
           "days-stats orchestrator semantic snapshot should count "
           "non-empty rows.",
           failures);
    Expect(kStatsObject->value("mean_seconds", -1.0) == 2700.0,
           "days-stats orchestrator semantic snapshot should keep mean "
           "contract.",
           failures);
  }

  DataQueryRequest report_chart_request;
  report_chart_request.action = DataQueryAction::kReportChart;
  report_chart_request.from_date = "2026-02-01";
  report_chart_request.to_date = "2026-02-03";
  report_chart_request.root = "study";
  const auto kChartOutput =
      time_tracer::infrastructure::query::data::orchestrators::
          HandleReportChartQuery(kDatabase.get(), report_chart_request, true);
  Expect(kChartOutput.ok, "report-chart orchestrator should succeed.",
         failures);
  if (!kChartOutput.ok) {
    return;
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
  Expect(kChartPayload.value("average_duration_seconds", -1LL) == 1800LL,
         "report-chart orchestrator semantic snapshot should keep average.",
         failures);
  Expect(kChartPayload.value("range_days", -1) == 3,
         "report-chart orchestrator semantic snapshot should keep range_days.",
         failures);

  DataQueryRequest missing_root_request = report_chart_request;
  missing_root_request.root = "nosuchroot";
  const auto kMissingRootOutput =
      time_tracer::infrastructure::query::data::orchestrators::
          HandleReportChartQuery(kDatabase.get(), missing_root_request, true);
  Expect(kMissingRootOutput.ok,
         "report-chart missing-root fallback should succeed.", failures);
  if (!kMissingRootOutput.ok) {
    return;
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
}

auto TestAdapterBoundaryGuardrails(int& failures) -> void {
  struct AdapterGuardRule {
    std::filesystem::path relative_path;
    std::vector<std::string> forbidden_tokens;
  };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();

  const std::vector<AdapterGuardRule> kRules = {
      {
          "apps/tracer_windows/src/api/cli/impl/commands/query/"
          "data_query_parser.cpp",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_windows/src/api/cli/impl/commands/query/"
          "query_command.cpp",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_windows/src/bootstrap/cli_runtime_factory_proxy.cpp",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_android/runtime/src/main/java/com/example/tracer/"
          "runtime/"
          "controller/RuntimeQueryDelegate.kt",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_android/runtime/src/main/java/com/example/tracer/"
          "runtime/"
          "NativeRuntimeQueryOps.kt",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/time_tracer/src/infrastructure/persistence/"
          "sqlite_data_query_service.cpp",
          {"std::sqrt(", "nearest-rank",
           "variance_seconds =", "stddev_seconds ="},
      },
      {
          "apps/time_tracer/src/infrastructure/persistence/"
          "sqlite_data_query_service_dispatch.cpp",
          {"std::sqrt(", "nearest-rank",
           "variance_seconds =", "stddev_seconds ="},
      },
      {
          "apps/time_tracer/src/infrastructure/persistence/"
          "sqlite_data_query_service_report_mapping.cpp",
          {"std::sqrt(", "nearest-rank",
           "variance_seconds =", "stddev_seconds ="},
      },
  };

  for (const auto& rule : kRules) {
    const std::filesystem::path target = kRepoRoot / rule.relative_path;
    const auto content = ReadFileText(target);
    Expect(
        content.has_value(),
        std::string("adapter guardrails should read file: ") + target.string(),
        failures);
    if (!content.has_value()) {
      continue;
    }
    for (const auto& token : rule.forbidden_tokens) {
      if (Contains(*content, token)) {
        ++failures;
        std::cerr << "[FAIL] adapter guardrail hit forbidden token `" << token
                  << "` in " << target.string() << '\n';
      }
    }
  }
}

}  // namespace

auto RunDataQueryRefactorTests(int& failures) -> void {
  TestDayDurationStatsCalculator(failures);
  TestReportChartSeriesCalculator(failures);
  TestSemanticDayStatsSnapshot(failures);
  TestDateRangeResolver(failures);
  TestRendererGateway(failures);
  TestOrchestratorRendererSemanticSnapshot(failures);
  TestAdapterBoundaryGuardrails(failures);
}

}  // namespace android_runtime_tests

// infrastructure/tests/android_runtime/android_runtime_smoke_bootstrap_tests.cpp
#include <exception>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

#include "infrastructure/tests/android_runtime/android_runtime_smoke_query_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

namespace android_runtime_tests::smoke {
namespace {

using nlohmann::json;

auto ExpectDataQueryFailureWithoutDb(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const tracer_core::core::dto::DataQueryRequest& request,
    const std::filesystem::path& db_path, std::string_view context,
    int& failures) -> void {
  const auto result = runtime_api->query().RunDataQuery(request);
  if (result.ok) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should fail when the database does not exist.\n";
  } else if (result.error_message.empty()) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should return a non-empty error message.\n";
  }

  if (std::filesystem::exists(db_path)) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should not create a database artifact.\n";
  }
}

auto ExpectReportQueryFailureWithoutDb(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const tracer_core::core::dto::ReportQueryRequest& request,
    const std::filesystem::path& db_path, std::string_view context,
    int& failures) -> void {
  const auto result = runtime_api->report().RunReportQuery(request);
  if (result.ok) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should fail when the database does not exist.\n";
  } else if (result.error_message.empty()) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should return a non-empty error message.\n";
  }

  if (std::filesystem::exists(db_path)) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should not create a database artifact.\n";
  }
}

auto ExpectStatsPeriodQueryFailureWithoutDb(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, std::string period,
    std::optional<std::string> period_argument,
    std::optional<int> lookback_days, const std::filesystem::path& db_path,
    int& failures) -> void {
  tracer_core::core::dto::DataQueryRequest request;
  request.action = tracer_core::core::dto::DataQueryAction::kDaysStats;
  request.tree_period = period;
  request.tree_period_argument = std::move(period_argument);
  request.lookback_days = lookback_days;

  const std::string context =
      "RunDataQuery(days-stats, period='" + period + "')";
  ExpectDataQueryFailureWithoutDb(runtime_api, request, db_path, context,
                                  failures);
}

}  // namespace

auto ParseJsonOrRecordFailure(const std::string& content,
                              std::string_view context, int& failures)
    -> std::optional<json> {
  try {
    return json::parse(content);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should return valid JSON payload: " << exception.what()
              << '\n';
  }
  return std::nullopt;
}

auto ValidateChartSeriesPayload(const json& payload, std::string_view context,
                                int& failures) -> void {
  if (!payload.contains("roots") || !payload["roots"].is_array()) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include array field `roots`.\n";
  }
  if (!payload.contains("series") || !payload["series"].is_array()) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include array field `series`.\n";
    return;
  }
  if (!payload.contains("lookback_days") ||
      !payload["lookback_days"].is_number_integer()) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include integer field `lookback_days`.\n";
  }
  const bool has_average_duration =
      payload.contains("average_duration_seconds") &&
      payload["average_duration_seconds"].is_number_integer();
  if (!has_average_duration) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include integer field "
                 "`average_duration_seconds`.\n";
  }
  const bool has_total_duration =
      payload.contains("total_duration_seconds") &&
      payload["total_duration_seconds"].is_number_integer();
  if (!has_total_duration) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include integer field "
                 "`total_duration_seconds`.\n";
  }
  const bool has_active_days = payload.contains("active_days") &&
                               payload["active_days"].is_number_integer();
  if (!has_active_days) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include integer field `active_days`.\n";
  }
  const bool has_range_days = payload.contains("range_days") &&
                              payload["range_days"].is_number_integer();
  if (!has_range_days) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " payload should include integer field `range_days`.\n";
  }

  std::string previous_date;
  std::optional<long long> previous_epoch_day;
  bool series_valid = true;
  long long total_duration_from_series = 0;
  int active_days_from_series = 0;
  int range_days_from_series = 0;
  for (const auto& row : payload["series"]) {
    if (!row.is_object()) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series row should be JSON object.\n";
      series_valid = false;
      break;
    }
    if (!row.contains("date") || !row["date"].is_string()) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series row should contain string field `date`.\n";
      series_valid = false;
      break;
    }
    if (!row.contains("duration_seconds") ||
        !row["duration_seconds"].is_number_integer()) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series row should contain integer field "
                   "`duration_seconds`.\n";
      series_valid = false;
      break;
    }
    if (!row.contains("epoch_day") || !row["epoch_day"].is_number_integer()) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series row should contain integer field `epoch_day`.\n";
      series_valid = false;
      break;
    }
    const std::string current_date = row["date"].get<std::string>();
    if (!previous_date.empty() && current_date < previous_date) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series should be sorted by date ascending.\n";
      series_valid = false;
      break;
    }
    const long long current_epoch_day = row["epoch_day"].get<long long>();
    if (previous_epoch_day.has_value() &&
        current_epoch_day != *previous_epoch_day + 1) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series epoch_day should be contiguous (+1 per point).\n";
      series_valid = false;
      break;
    }
    const long long duration_seconds = row["duration_seconds"].get<long long>();
    total_duration_from_series += duration_seconds;
    ++range_days_from_series;
    if (duration_seconds > 0) {
      ++active_days_from_series;
    }
    previous_date = current_date;
    previous_epoch_day = current_epoch_day;
  }

  if (series_valid && has_average_duration && has_total_duration &&
      has_active_days && has_range_days) {
    const long long total_duration_in_payload =
        payload["total_duration_seconds"].get<long long>();
    const long long average_duration_in_payload =
        payload["average_duration_seconds"].get<long long>();
    const int active_days_in_payload = payload["active_days"].get<int>();
    const int range_days_in_payload = payload["range_days"].get<int>();

    if (range_days_in_payload != range_days_from_series) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " payload `range_days` should match series size.\n";
    }
    if (total_duration_in_payload != total_duration_from_series) {
      ++failures;
      std::cerr
          << "[FAIL] " << context
          << " payload `total_duration_seconds` should match series sum.\n";
    }
    if (active_days_in_payload != active_days_from_series) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " payload `active_days` should match count(duration > 0).\n";
    }
    if (active_days_in_payload > range_days_in_payload) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " payload `active_days` should be <= `range_days`.\n";
    }

    const long long expected_average_duration =
        range_days_in_payload > 0
            ? (total_duration_in_payload /
               static_cast<long long>(range_days_in_payload))
            : 0LL;
    if (average_duration_in_payload != expected_average_duration) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " payload `average_duration_seconds` should be "
                   "total/range (integer division).\n";
    }
  }
}

auto RunDataQueryOrRecordFailure(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const tracer_core::core::dto::DataQueryRequest& request,
    std::string_view context, int& failures)
    -> std::optional<tracer_core::core::dto::TextOutput> {
  const auto result = runtime_api->query().RunDataQuery(request);
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should succeed: " << result.error_message << '\n';
    return std::nullopt;
  }
  return result;
}

auto RunBootstrapSmokeSection(int& failures) -> void {
  auto fixture_opt = BuildRuntimeFixture(
      "time_tracer_android_runtime_smoke_bootstrap_test", failures);
  if (!fixture_opt.has_value()) {
    return;
  }

  RuntimeFixture fixture = std::move(*fixture_opt);
  try {
    if (std::filesystem::exists(fixture.paths.db_path)) {
      ++failures;
      std::cerr << "[FAIL] Fresh runtime bootstrap should not create a "
                   "database file.\n";
    }

    tracer_core::core::dto::DataQueryRequest years_request;
    years_request.action = tracer_core::core::dto::DataQueryAction::kYears;
    ExpectDataQueryFailureWithoutDb(fixture.runtime.runtime_api, years_request,
                                    fixture.paths.db_path,
                                    "RunDataQuery(years)", failures);

    tracer_core::core::dto::DataQueryRequest mapping_names_request;
    mapping_names_request.action =
        tracer_core::core::dto::DataQueryAction::kMappingNames;
    if (const auto mapping_names_result = RunDataQueryOrRecordFailure(
            fixture.runtime.runtime_api, mapping_names_request,
            "RunDataQuery(mapping-names)", failures);
        mapping_names_result.has_value() &&
        !Contains(mapping_names_result->content, "\"names\"")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(mapping-names) output should include "
                   "\"names\" JSON key.\n";
    }
    if (std::filesystem::exists(fixture.paths.db_path)) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(mapping-names) should not create a "
                   "database file.\n";
    }

    tracer_core::core::dto::DataQueryRequest stats_request;
    stats_request.action = tracer_core::core::dto::DataQueryAction::kDaysStats;
    ExpectDataQueryFailureWithoutDb(fixture.runtime.runtime_api, stats_request,
                                    fixture.paths.db_path,
                                    "RunDataQuery(days-stats)", failures);

    ExpectStatsPeriodQueryFailureWithoutDb(
        fixture.runtime.runtime_api, "day", std::string("2026-02-01"),
        std::nullopt, fixture.paths.db_path, failures);
    ExpectStatsPeriodQueryFailureWithoutDb(
        fixture.runtime.runtime_api, "week", std::string("2026-W05"),
        std::nullopt, fixture.paths.db_path, failures);
    ExpectStatsPeriodQueryFailureWithoutDb(fixture.runtime.runtime_api, "month",
                                           std::string("2026-02"), std::nullopt,
                                           fixture.paths.db_path, failures);
    ExpectStatsPeriodQueryFailureWithoutDb(fixture.runtime.runtime_api, "year",
                                           std::string("2026"), std::nullopt,
                                           fixture.paths.db_path, failures);
    ExpectStatsPeriodQueryFailureWithoutDb(
        fixture.runtime.runtime_api, "recent", std::string("7"), std::nullopt,
        fixture.paths.db_path, failures);
    ExpectStatsPeriodQueryFailureWithoutDb(fixture.runtime.runtime_api,
                                           "recent", std::nullopt, 7,
                                           fixture.paths.db_path, failures);
    ExpectStatsPeriodQueryFailureWithoutDb(fixture.runtime.runtime_api, "range",
                                           std::string("2026-02-01|2026-02-15"),
                                           std::nullopt, fixture.paths.db_path,
                                           failures);

    tracer_core::core::dto::DataQueryRequest invalid_stats_period_request;
    invalid_stats_period_request.action =
        tracer_core::core::dto::DataQueryAction::kDaysStats;
    invalid_stats_period_request.tree_period = "invalid-period";
    invalid_stats_period_request.tree_period_argument = "1";
    const auto invalid_stats_period_result =
        fixture.runtime.runtime_api->query().RunDataQuery(
            invalid_stats_period_request);
    if (invalid_stats_period_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) should reject invalid "
                   "period value.\n";
    }

    tracer_core::core::dto::DataQueryRequest tree_request;
    tree_request.action = tracer_core::core::dto::DataQueryAction::kTree;
    tree_request.tree_period = "recent";
    tree_request.tree_period_argument = "7";
    tree_request.tree_max_depth = 1;
    ExpectDataQueryFailureWithoutDb(fixture.runtime.runtime_api, tree_request,
                                    fixture.paths.db_path, "RunDataQuery(tree)",
                                    failures);

    ExpectReportQueryFailureWithoutDb(
        fixture.runtime.runtime_api,
        {.type = tracer_core::core::dto::ReportQueryType::kRecent,
         .argument = "1",
         .format = ReportFormat::kMarkdown},
        fixture.paths.db_path, "RunReportQuery(recent)", failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Android runtime bootstrap smoke test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr
        << "[FAIL] Android runtime bootstrap smoke test threw non-standard "
           "exception.\n";
  }

  CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests::smoke

namespace android_runtime_tests {

auto RunRuntimeSmokeTests(int& failures) -> void {
  smoke::RunBootstrapSmokeSection(failures);
  smoke::RunConfigSmokeSection(failures);
  smoke::RunIoSmokeSection(failures);
}

}  // namespace android_runtime_tests

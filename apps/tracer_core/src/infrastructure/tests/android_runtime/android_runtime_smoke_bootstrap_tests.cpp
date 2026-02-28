// infrastructure/tests/android_runtime/android_runtime_smoke_bootstrap_tests.cpp
#include <exception>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

namespace android_runtime_tests::smoke {
namespace {

using nlohmann::json;

auto RunStatsPeriodQuery(const std::shared_ptr<ITracerCoreApi>& core_api,
                         std::string period,
                         std::optional<std::string> period_argument,
                         std::optional<int> lookback_days, int& failures)
    -> void {
  tracer_core::core::dto::DataQueryRequest request;
  request.action = tracer_core::core::dto::DataQueryAction::kDaysStats;
  request.tree_period = period;
  request.tree_period_argument = std::move(period_argument);
  request.lookback_days = lookback_days;

  const std::string context =
      "RunDataQuery(days-stats, period='" + period + "')";
  const auto result =
      RunDataQueryOrRecordFailure(core_api, request, context, failures);
  if (!result.has_value()) {
    return;
  }
  if (!Contains(result->content, "## Day Duration Stats")) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " output should include '## Day Duration Stats'.\n";
  }
  if (!Contains(result->content, "**Days**: 0")) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " output should include '**Days**: 0'.\n";
  }
}

}  // namespace

auto BuildRuntimeFixture(std::string_view test_name, int& failures)
    -> std::optional<RuntimeFixture> {
  RuntimeFixture fixture;
  fixture.paths = BuildTempTestPaths(test_name);

  const std::filesystem::path repo_root = BuildRepoRoot();
  fixture.input_path = repo_root / "test" / "data";
  fixture.config_toml_path = repo_root / "apps" / "tracer_core" / "config" /
                             "converter" / "interval_processor_config.toml";

  RemoveTree(fixture.paths.test_root);

  try {
    const auto request =
        BuildRuntimeRequest(fixture.paths, fixture.config_toml_path);
    fixture.runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should not throw: "
              << exception.what() << '\n';
    RemoveTree(fixture.paths.test_root);
    return std::nullopt;
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should not throw non-standard "
                 "exception.\n";
    RemoveTree(fixture.paths.test_root);
    return std::nullopt;
  }

  if (!fixture.runtime.core_api) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should return a valid core API.\n";
    RemoveTree(fixture.paths.test_root);
    return std::nullopt;
  }

  return fixture;
}

auto CleanupRuntimeFixture(const RuntimeFixture& fixture) -> void {
  RemoveTree(fixture.paths.test_root);
}

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
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const tracer_core::core::dto::DataQueryRequest& request,
    std::string_view context, int& failures)
    -> std::optional<tracer_core::core::dto::TextOutput> {
  const auto result = core_api->RunDataQuery(request);
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
    tracer_core::core::dto::DataQueryRequest years_request;
    years_request.action = tracer_core::core::dto::DataQueryAction::kYears;
    if (const auto years_result =
            RunDataQueryOrRecordFailure(fixture.runtime.core_api, years_request,
                                        "RunDataQuery(years)", failures);
        years_result.has_value() &&
        !Contains(years_result->content, "Total: 0")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(years) output should include "
                   "'Total: 0'.\n";
    }

    tracer_core::core::dto::DataQueryRequest mapping_names_request;
    mapping_names_request.action =
        tracer_core::core::dto::DataQueryAction::kMappingNames;
    if (const auto mapping_names_result = RunDataQueryOrRecordFailure(
            fixture.runtime.core_api, mapping_names_request,
            "RunDataQuery(mapping-names)", failures);
        mapping_names_result.has_value() &&
        !Contains(mapping_names_result->content, "\"names\"")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(mapping-names) output should include "
                   "\"names\" JSON key.\n";
    }

    tracer_core::core::dto::DataQueryRequest stats_request;
    stats_request.action = tracer_core::core::dto::DataQueryAction::kDaysStats;
    if (const auto stats_result =
            RunDataQueryOrRecordFailure(fixture.runtime.core_api, stats_request,
                                        "RunDataQuery(days-stats)", failures);
        stats_result.has_value()) {
      if (!Contains(stats_result->content, "## Day Duration Stats")) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats) output should include "
                     "'## Day Duration Stats'.\n";
      }
      if (!Contains(stats_result->content, "**Days**: 0")) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats) output should include "
                     "'**Days**: 0'.\n";
      }
    }

    RunStatsPeriodQuery(fixture.runtime.core_api, "day",
                        std::string("2026-02-01"), std::nullopt, failures);
    RunStatsPeriodQuery(fixture.runtime.core_api, "week",
                        std::string("2026-W05"), std::nullopt, failures);
    RunStatsPeriodQuery(fixture.runtime.core_api, "month",
                        std::string("2026-02"), std::nullopt, failures);
    RunStatsPeriodQuery(fixture.runtime.core_api, "year", std::string("2026"),
                        std::nullopt, failures);
    RunStatsPeriodQuery(fixture.runtime.core_api, "recent", std::string("7"),
                        std::nullopt, failures);
    RunStatsPeriodQuery(fixture.runtime.core_api, "recent", std::nullopt, 7,
                        failures);
    RunStatsPeriodQuery(fixture.runtime.core_api, "range",
                        std::string("2026-02-01|2026-02-15"), std::nullopt,
                        failures);

    tracer_core::core::dto::DataQueryRequest invalid_stats_period_request;
    invalid_stats_period_request.action =
        tracer_core::core::dto::DataQueryAction::kDaysStats;
    invalid_stats_period_request.tree_period = "invalid-period";
    invalid_stats_period_request.tree_period_argument = "1";
    const auto invalid_stats_period_result =
        fixture.runtime.core_api->RunDataQuery(invalid_stats_period_request);
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
    if (const auto tree_result =
            RunDataQueryOrRecordFailure(fixture.runtime.core_api, tree_request,
                                        "RunDataQuery(tree)", failures);
        tree_result.has_value() &&
        !Contains(tree_result->content, "Total: 0")) {
      ++failures;
      std::cerr
          << "[FAIL] RunDataQuery(tree) output should include 'Total: 0'.\n";
    }
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

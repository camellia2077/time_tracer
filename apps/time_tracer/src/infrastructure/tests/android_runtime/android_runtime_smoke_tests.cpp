// infrastructure/tests/android_runtime_smoke_tests.cpp
#include <sqlite3.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

using nlohmann::json;

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
    const std::string current_date = row["date"].get<std::string>();
    if (!previous_date.empty() && current_date < previous_date) {
      ++failures;
      std::cerr << "[FAIL] " << context
                << " series should be sorted by date ascending.\n";
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
      std::cerr << "[FAIL] " << context
                << " payload `total_duration_seconds` should match series "
                   "sum.\n";
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

auto TestAndroidRuntimeCanRunDataQuery(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_android_runtime_factory_test");
  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kInputPath = kRepoRoot / "test" / "data";
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "apps" / "time_tracer" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid core API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    time_tracer::core::dto::DataQueryRequest query_request;
    query_request.action = time_tracer::core::dto::DataQueryAction::kYears;

    const auto query_result = runtime.core_api->RunDataQuery(query_request);
    if (!query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery should succeed: "
                << query_result.error_message << '\n';
    }
    if (!Contains(query_result.content, "Total: 0")) {
      ++failures;
      std::cerr
          << "[FAIL] RunDataQuery years output should include 'Total: 0'.\n";
    }

    time_tracer::core::dto::DataQueryRequest mapping_names_request;
    mapping_names_request.action =
        time_tracer::core::dto::DataQueryAction::kMappingNames;
    const auto mapping_names_result =
        runtime.core_api->RunDataQuery(mapping_names_request);
    if (!mapping_names_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(mapping-names) should succeed: "
                << mapping_names_result.error_message << '\n';
    } else if (!Contains(mapping_names_result.content, "\"names\"")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(mapping-names) output should include "
                   "\"names\" JSON key.\n";
    }

    time_tracer::core::dto::DataQueryRequest stats_query_request;
    stats_query_request.action =
        time_tracer::core::dto::DataQueryAction::kDaysStats;
    const auto stats_query_result =
        runtime.core_api->RunDataQuery(stats_query_request);
    if (!stats_query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) should succeed: "
                << stats_query_result.error_message << '\n';
    } else if (!Contains(stats_query_result.content, "## Day Duration Stats")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) output should include "
                   "'## Day Duration Stats'.\n";
    } else if (!Contains(stats_query_result.content, "**Days**: 0")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) output should include "
                   "'**Days**: 0'.\n";
    }

    const auto run_stats_period =
        [&](std::string period, std::optional<std::string> period_argument,
            std::optional<int> lookback_days) -> void {
      time_tracer::core::dto::DataQueryRequest request;
      request.action = time_tracer::core::dto::DataQueryAction::kDaysStats;
      request.tree_period = std::move(period);
      request.tree_period_argument = std::move(period_argument);
      request.lookback_days = lookback_days;

      const auto result = runtime.core_api->RunDataQuery(request);
      if (!result.ok) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats, period='"
                  << *request.tree_period
                  << "') should succeed: " << result.error_message << '\n';
      } else if (!Contains(result.content, "## Day Duration Stats")) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats, period='"
                  << *request.tree_period
                  << "') output should include '## Day Duration Stats'.\n";
      } else if (!Contains(result.content, "**Days**: 0")) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats, period='"
                  << *request.tree_period
                  << "') output should include '**Days**: 0'.\n";
      }
    };

    run_stats_period("day", std::string("2026-02-01"), std::nullopt);
    run_stats_period("week", std::string("2026-W05"), std::nullopt);
    run_stats_period("month", std::string("2026-02"), std::nullopt);
    run_stats_period("year", std::string("2026"), std::nullopt);
    run_stats_period("recent", std::string("7"), std::nullopt);
    run_stats_period("recent", std::nullopt, 7);
    run_stats_period("range", std::string("2026-02-01|2026-02-15"),
                     std::nullopt);

    time_tracer::core::dto::DataQueryRequest invalid_stats_period_request;
    invalid_stats_period_request.action =
        time_tracer::core::dto::DataQueryAction::kDaysStats;
    invalid_stats_period_request.tree_period = "invalid-period";
    invalid_stats_period_request.tree_period_argument = "1";
    const auto invalid_stats_period_result =
        runtime.core_api->RunDataQuery(invalid_stats_period_request);
    if (invalid_stats_period_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) should reject invalid "
                   "period value.\n";
    }

    time_tracer::core::dto::DataQueryRequest tree_query_request;
    tree_query_request.action = time_tracer::core::dto::DataQueryAction::kTree;
    tree_query_request.tree_period = "recent";
    tree_query_request.tree_period_argument = "7";
    tree_query_request.tree_max_depth = 1;
    const auto tree_query_result =
        runtime.core_api->RunDataQuery(tree_query_request);
    if (!tree_query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(tree) should succeed: "
                << tree_query_result.error_message << '\n';
    } else if (!Contains(tree_query_result.content, "Total: 0")) {
      ++failures;
      std::cerr
          << "[FAIL] RunDataQuery(tree) output should include 'Total: 0'.\n";
    }

    time_tracer::core::dto::DataQueryRequest chart_empty_request;
    chart_empty_request.action =
        time_tracer::core::dto::DataQueryAction::kReportChart;
    chart_empty_request.lookback_days = 7;
    const auto chart_empty_result =
        runtime.core_api->RunDataQuery(chart_empty_request);
    if (!chart_empty_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart, empty) should succeed: "
                << chart_empty_result.error_message << '\n';
    } else if (const auto payload = ParseJsonOrRecordFailure(
                   chart_empty_result.content, "report-chart empty", failures);
               payload.has_value()) {
      ValidateChartSeriesPayload(*payload, "report-chart empty", failures);
      const auto roots_it = payload->find("roots");
      if (roots_it != payload->end() && roots_it->is_array() &&
          !roots_it->empty()) {
        ++failures;
        std::cerr << "[FAIL] report-chart empty payload roots should be "
                     "empty.\n";
      }
      const auto series_it = payload->find("series");
      if (series_it != payload->end() && series_it->is_array() &&
          !series_it->empty()) {
        ++failures;
        std::cerr << "[FAIL] report-chart empty payload series should be "
                     "empty.\n";
      }
    }

    time_tracer::core::dto::DataQueryRequest chart_invalid_request;
    chart_invalid_request.action =
        time_tracer::core::dto::DataQueryAction::kReportChart;
    chart_invalid_request.lookback_days = 0;
    const auto chart_invalid_result =
        runtime.core_api->RunDataQuery(chart_invalid_request);
    if (chart_invalid_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart) should reject "
                   "lookback_days <= 0.\n";
    } else if (!Contains(chart_invalid_result.error_message,
                         "--lookback-days")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart) invalid lookback error "
                   "should mention --lookback-days.\n";
    }

    time_tracer::core::dto::DataQueryRequest
        chart_invalid_range_missing_request;
    chart_invalid_range_missing_request.action =
        time_tracer::core::dto::DataQueryAction::kReportChart;
    chart_invalid_range_missing_request.from_date = "2026-02-01";
    const auto chart_invalid_range_missing_result =
        runtime.core_api->RunDataQuery(chart_invalid_range_missing_request);
    if (chart_invalid_range_missing_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart) should reject missing "
                   "to_date when from_date is provided.\n";
    }

    time_tracer::core::dto::DataQueryRequest chart_invalid_range_order_request;
    chart_invalid_range_order_request.action =
        time_tracer::core::dto::DataQueryAction::kReportChart;
    chart_invalid_range_order_request.from_date = "2026-02-15";
    chart_invalid_range_order_request.to_date = "2026-02-01";
    const auto chart_invalid_range_order_result =
        runtime.core_api->RunDataQuery(chart_invalid_range_order_request);
    if (chart_invalid_range_order_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart) should reject "
                   "from_date > to_date.\n";
    }

    time_tracer::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = kInputPath.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result = runtime.core_api->RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunIngest should succeed for report-chart test: "
                << ingest_result.error_message << '\n';
    } else {
      time_tracer::core::dto::DataQueryRequest chart_range_request;
      chart_range_request.action =
          time_tracer::core::dto::DataQueryAction::kReportChart;
      chart_range_request.lookback_days = 7;

      std::optional<std::string> latest_date;
      std::optional<std::string> earliest_date;
      std::optional<std::string> selected_root;

      const auto chart_range_result =
          runtime.core_api->RunDataQuery(chart_range_request);
      if (!chart_range_result.ok) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(report-chart, range) should "
                     "succeed: "
                  << chart_range_result.error_message << '\n';
      } else if (const auto payload =
                     ParseJsonOrRecordFailure(chart_range_result.content,
                                              "report-chart range", failures);
                 payload.has_value()) {
        ValidateChartSeriesPayload(*payload, "report-chart range", failures);
        const auto series_it = payload->find("series");
        if (series_it == payload->end() || !series_it->is_array() ||
            series_it->size() != 7U) {
          ++failures;
          std::cerr << "[FAIL] report-chart range should return 7 points for "
                       "lookback_days=7.\n";
        }
        const auto roots_it = payload->find("roots");
        if (roots_it == payload->end() || !roots_it->is_array() ||
            roots_it->empty()) {
          ++failures;
          std::cerr << "[FAIL] report-chart range roots should not be empty "
                       "after ingest.\n";
        } else {
          const auto& roots = *roots_it;
          if (roots.front().is_string()) {
            selected_root = roots.front().get<std::string>();
          }
          for (size_t i = 1; i < roots.size(); ++i) {
            if (roots[i - 1].is_string() && roots[i].is_string() &&
                roots[i].get<std::string>() < roots[i - 1].get<std::string>()) {
              ++failures;
              std::cerr << "[FAIL] report-chart roots should be sorted "
                           "ascending.\n";
              break;
            }
          }
        }
        if (series_it != payload->end() && series_it->is_array() &&
            !series_it->empty()) {
          const auto& first = series_it->front();
          if (first.is_object() && first.contains("date") &&
              first["date"].is_string()) {
            earliest_date = first["date"].get<std::string>();
          }
          const auto& last = series_it->back();
          if (last.is_object() && last.contains("date") &&
              last["date"].is_string()) {
            latest_date = last["date"].get<std::string>();
          }
        }
      }

      if (earliest_date.has_value() && latest_date.has_value()) {
        time_tracer::core::dto::DataQueryRequest chart_explicit_range_request;
        chart_explicit_range_request.action =
            time_tracer::core::dto::DataQueryAction::kReportChart;
        chart_explicit_range_request.from_date = *earliest_date;
        chart_explicit_range_request.to_date = *latest_date;
        const auto chart_explicit_range_result =
            runtime.core_api->RunDataQuery(chart_explicit_range_request);
        if (!chart_explicit_range_result.ok) {
          ++failures;
          std::cerr << "[FAIL] RunDataQuery(report-chart, explicit range) "
                       "should succeed: "
                    << chart_explicit_range_result.error_message << '\n';
        } else if (const auto payload = ParseJsonOrRecordFailure(
                       chart_explicit_range_result.content,
                       "report-chart explicit range", failures);
                   payload.has_value()) {
          ValidateChartSeriesPayload(*payload, "report-chart explicit range",
                                     failures);
          const auto series_it = payload->find("series");
          if (series_it == payload->end() || !series_it->is_array() ||
              series_it->size() != 7U) {
            ++failures;
            std::cerr << "[FAIL] report-chart explicit range should return 7 "
                         "points.\n";
          }
          if (payload->value("from_date", std::string()) != *earliest_date ||
              payload->value("to_date", std::string()) != *latest_date) {
            ++failures;
            std::cerr << "[FAIL] report-chart explicit range should echo "
                         "resolved from_date/to_date.\n";
          }
        }
      }

      if (selected_root.has_value()) {
        time_tracer::core::dto::DataQueryRequest chart_root_request;
        chart_root_request.action =
            time_tracer::core::dto::DataQueryAction::kReportChart;
        chart_root_request.lookback_days = 7;
        chart_root_request.root = *selected_root;
        const auto chart_root_result =
            runtime.core_api->RunDataQuery(chart_root_request);
        if (!chart_root_result.ok) {
          ++failures;
          std::cerr << "[FAIL] RunDataQuery(report-chart, selected root) "
                       "should succeed: "
                    << chart_root_result.error_message << '\n';
        } else if (const auto payload = ParseJsonOrRecordFailure(
                       chart_root_result.content, "report-chart selected root",
                       failures);
                   payload.has_value()) {
          ValidateChartSeriesPayload(*payload, "report-chart selected root",
                                     failures);
          const auto series_it = payload->find("series");
          if (series_it == payload->end() || !series_it->is_array() ||
              series_it->size() != 7U) {
            ++failures;
            std::cerr << "[FAIL] report-chart selected root should return 7 "
                         "points.\n";
          }
          if (payload->value("selected_root", std::string()) !=
              *selected_root) {
            ++failures;
            std::cerr << "[FAIL] report-chart selected_root should match "
                         "request --root value.\n";
          }
        }

        time_tracer::core::dto::DataQueryRequest chart_root_overrides_project;
        chart_root_overrides_project.action =
            time_tracer::core::dto::DataQueryAction::kReportChart;
        chart_root_overrides_project.lookback_days = 7;
        chart_root_overrides_project.root = *selected_root;
        chart_root_overrides_project.project = "zzzz_override_project";
        const auto chart_root_overrides_project_result =
            runtime.core_api->RunDataQuery(chart_root_overrides_project);
        if (!chart_root_overrides_project_result.ok) {
          ++failures;
          std::cerr << "[FAIL] RunDataQuery(report-chart, root overrides "
                       "project) should succeed: "
                    << chart_root_overrides_project_result.error_message
                    << '\n';
        } else if (const auto payload = ParseJsonOrRecordFailure(
                       chart_root_overrides_project_result.content,
                       "report-chart root overrides project", failures);
                   payload.has_value()) {
          ValidateChartSeriesPayload(
              *payload, "report-chart root overrides project", failures);
          if (payload->value("selected_root", std::string()) !=
              *selected_root) {
            ++failures;
            std::cerr
                << "[FAIL] report-chart should prefer request --root over "
                   "--project when both are provided.\n";
          }
        }

        const std::string kMissingRoot = "nosuchroot";
        time_tracer::core::dto::DataQueryRequest chart_missing_root_request;
        chart_missing_root_request.action =
            time_tracer::core::dto::DataQueryAction::kReportChart;
        chart_missing_root_request.lookback_days = 7;
        chart_missing_root_request.root = kMissingRoot;
        const auto chart_missing_root_result =
            runtime.core_api->RunDataQuery(chart_missing_root_request);
        if (!chart_missing_root_result.ok) {
          ++failures;
          std::cerr << "[FAIL] RunDataQuery(report-chart, missing root) "
                       "should succeed: "
                    << chart_missing_root_result.error_message << '\n';
        } else if (const auto payload = ParseJsonOrRecordFailure(
                       chart_missing_root_result.content,
                       "report-chart missing root", failures);
                   payload.has_value()) {
          ValidateChartSeriesPayload(*payload, "report-chart missing root",
                                     failures);
          if (payload->value("selected_root", std::string()) != kMissingRoot) {
            ++failures;
            std::cerr << "[FAIL] report-chart missing root should echo "
                         "selected_root from request.\n";
          }
          if (payload->value("total_duration_seconds", -1LL) != 0LL) {
            ++failures;
            std::cerr << "[FAIL] report-chart missing root should produce "
                         "total_duration_seconds=0.\n";
          }
          if (payload->value("active_days", -1) != 0) {
            ++failures;
            std::cerr << "[FAIL] report-chart missing root should produce "
                         "active_days=0.\n";
          }
        }

        time_tracer::core::dto::DataQueryRequest chart_empty_root_request;
        chart_empty_root_request.action =
            time_tracer::core::dto::DataQueryAction::kReportChart;
        chart_empty_root_request.lookback_days = 7;
        chart_empty_root_request.root = "   ";
        const auto chart_empty_root_result =
            runtime.core_api->RunDataQuery(chart_empty_root_request);
        if (!chart_empty_root_result.ok) {
          ++failures;
          std::cerr << "[FAIL] RunDataQuery(report-chart, empty root) should "
                       "succeed: "
                    << chart_empty_root_result.error_message << '\n';
        } else if (const auto payload = ParseJsonOrRecordFailure(
                       chart_empty_root_result.content,
                       "report-chart empty root", failures);
                   payload.has_value()) {
          ValidateChartSeriesPayload(*payload, "report-chart empty root",
                                     failures);
          if (!payload->value("selected_root", std::string()).empty()) {
            ++failures;
            std::cerr << "[FAIL] report-chart empty root should normalize to "
                         "empty selected_root.\n";
          }
        }

        time_tracer::core::dto::DataQueryRequest chart_boundary_request;
        chart_boundary_request.action =
            time_tracer::core::dto::DataQueryAction::kReportChart;
        chart_boundary_request.root = *selected_root;
        chart_boundary_request.lookback_days = 1;
        const auto chart_boundary_result =
            runtime.core_api->RunDataQuery(chart_boundary_request);
        if (!chart_boundary_result.ok) {
          ++failures;
          std::cerr << "[FAIL] RunDataQuery(report-chart, boundary) should "
                       "succeed: "
                    << chart_boundary_result.error_message << '\n';
        } else if (const auto payload = ParseJsonOrRecordFailure(
                       chart_boundary_result.content, "report-chart boundary",
                       failures);
                   payload.has_value()) {
          ValidateChartSeriesPayload(*payload, "report-chart boundary",
                                     failures);
          const auto series_it = payload->find("series");
          if (series_it == payload->end() || !series_it->is_array() ||
              series_it->size() != 1U) {
            ++failures;
            std::cerr << "[FAIL] report-chart boundary should return exactly "
                         "1 point for lookback_days=1.\n";
          }
          if (latest_date.has_value() && series_it != payload->end() &&
              series_it->is_array() && !series_it->empty()) {
            const auto& first = series_it->front();
            if (first.is_object() && first.contains("date") &&
                first["date"].is_string() &&
                first["date"].get<std::string>() != *latest_date) {
              ++failures;
              std::cerr << "[FAIL] report-chart boundary point date should be "
                           "the latest date.\n";
            }
          }
        }

        if (latest_date.has_value()) {
          time_tracer::core::dto::DataQueryRequest stats_base_request;
          stats_base_request.action =
              time_tracer::core::dto::DataQueryAction::kDaysStats;
          stats_base_request.tree_period = "day";
          stats_base_request.tree_period_argument = *latest_date;

          const auto stats_base_result =
              runtime.core_api->RunDataQuery(stats_base_request);
          if (!stats_base_result.ok) {
            ++failures;
            std::cerr << "[FAIL] RunDataQuery(days-stats, baseline day) "
                         "should succeed: "
                      << stats_base_result.error_message << '\n';
          } else if (!Contains(stats_base_result.content,
                               "## Day Duration Stats")) {
            ++failures;
            std::cerr << "[FAIL] days-stats baseline output should include "
                         "'## Day Duration Stats'.\n";
          }

          time_tracer::core::dto::DataQueryRequest stats_empty_root_request =
              stats_base_request;
          stats_empty_root_request.root = "   ";
          const auto stats_empty_root_result =
              runtime.core_api->RunDataQuery(stats_empty_root_request);
          if (!stats_empty_root_result.ok) {
            ++failures;
            std::cerr << "[FAIL] RunDataQuery(days-stats, empty root) should "
                         "succeed: "
                      << stats_empty_root_result.error_message << '\n';
          } else if (stats_base_result.ok && stats_empty_root_result.content !=
                                                 stats_base_result.content) {
            ++failures;
            std::cerr << "[FAIL] days-stats empty root should behave like "
                         "no root filter.\n";
          }

          time_tracer::core::dto::DataQueryRequest stats_valid_root_request =
              stats_base_request;
          stats_valid_root_request.root = *selected_root;
          const auto stats_valid_root_result =
              runtime.core_api->RunDataQuery(stats_valid_root_request);
          if (!stats_valid_root_result.ok) {
            ++failures;
            std::cerr << "[FAIL] RunDataQuery(days-stats, selected root) "
                         "should succeed: "
                      << stats_valid_root_result.error_message << '\n';
          } else if (!Contains(stats_valid_root_result.content,
                               "## Day Duration Stats")) {
            ++failures;
            std::cerr << "[FAIL] days-stats selected root output should "
                         "include '## Day Duration Stats'.\n";
          }

          time_tracer::core::dto::DataQueryRequest stats_missing_root_request =
              stats_base_request;
          stats_missing_root_request.root = kMissingRoot;
          const auto stats_missing_root_result =
              runtime.core_api->RunDataQuery(stats_missing_root_request);
          if (!stats_missing_root_result.ok) {
            ++failures;
            std::cerr << "[FAIL] RunDataQuery(days-stats, missing root) "
                         "should succeed: "
                      << stats_missing_root_result.error_message << '\n';
          } else if (!Contains(stats_missing_root_result.content,
                               "**Days**: 0")) {
            ++failures;
            std::cerr << "[FAIL] days-stats missing root output should "
                         "include '**Days**: 0'.\n";
          }
        }
      }
    }

    const auto report_result = RunAndCheckReportQuery(
        runtime.core_api,
        {.type = time_tracer::core::dto::ReportQueryType::kRecent,
         .argument = "1",
         .format = ReportFormat::kMarkdown},
        "markdown", failures);
    if (report_result && report_result->content.empty()) {
      ++failures;
      std::cerr << "[FAIL] RunReportQuery(markdown) should return non-empty "
                   "content.\n";
    }

    const auto day_report_result = RunAndCheckReportQuery(
        runtime.core_api,
        {.type = time_tracer::core::dto::ReportQueryType::kDay,
         .argument = "2026-02-01",
         .format = ReportFormat::kMarkdown},
        "day markdown", failures);
    if (day_report_result) {
      if (!Contains(day_report_result->content, "- **Date**: ")) {
        ++failures;
        std::cerr << "[FAIL] Android day markdown report should include "
                     "'Date' label.\n";
      }
      if (!Contains(day_report_result->content,
                    "- **Total Time Recorded**: ")) {
        ++failures;
        std::cerr << "[FAIL] Android day markdown report should include "
                     "'Total Time Recorded' label.\n";
      }
    }

    const auto structured_result = runtime.core_api->RunStructuredReportQuery(
        {.type = time_tracer::core::dto::ReportQueryType::kRecent,
         .argument = "1"});
    if (!structured_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunStructuredReportQuery should succeed: "
                << structured_result.error_message << '\n';
    }
    if (structured_result.kind !=
        time_tracer::core::dto::StructuredReportKind::kRecent) {
      ++failures;
      std::cerr << "[FAIL] RunStructuredReportQuery should return kRecent.\n";
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Android runtime test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Android runtime test threw non-standard exception.\n";
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunRuntimeSmokeTests(int& failures) -> void {
  TestAndroidRuntimeCanRunDataQuery(failures);
}

}  // namespace android_runtime_tests

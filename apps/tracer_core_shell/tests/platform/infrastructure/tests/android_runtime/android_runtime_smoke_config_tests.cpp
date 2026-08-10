// infrastructure/tests/android_runtime/android_runtime_smoke_config_tests.cpp
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include "application/dto/pipeline_requests.hpp"
#include "host/bootstrap/android_runtime_config_bridge.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

namespace android_runtime_tests::smoke {

auto RunConfigSmokeSection(int& failures) -> void {
  auto fixture_opt = BuildRuntimeFixture(
      "time_tracer_android_runtime_smoke_config_test", failures);
  if (!fixture_opt.has_value()) {
    return;
  }

  RuntimeFixture fixture = std::move(*fixture_opt);
  try {
    const auto runtime_config_paths =
        tracer_core::shell::config_bridge::
            ResolveAndroidRuntimeConfigPathsBridge(fixture.config_toml_path);
    const auto insights_catalog =
        tracer_core::shell::config_bridge::BuildAndroidInsightsCatalogBridge(
            runtime_config_paths);
    for (const std::string_view locale : {"en", "zh", "ja"}) {
      if (!insights_catalog.loaded_insights.markdown_locales.contains(
              std::string(locale))) {
        ++failures;
        std::cerr << "[FAIL] Android insights catalog should load Markdown "
                     "locale: "
                  << locale << '\n';
      }
    }
    const auto zh_it = insights_catalog.loaded_insights.markdown_locales.find("zh");
    if (zh_it == insights_catalog.loaded_insights.markdown_locales.end() ||
        zh_it->second.day.labels.date_label != "日期") {
      ++failures;
      std::cerr << "[FAIL] Android insights catalog should load Chinese Markdown "
                   "labels.\n";
    }
    if (insights_catalog.daily_statuses.statuses.size() != 2U) {
      ++failures;
      std::cerr << "[FAIL] Android insights catalog should load the configured "
                   "daily parent statuses.\n";
    }

    tracer_core::core::dto::DataQueryRequest chart_empty_request;
    chart_empty_request.action =
        tracer_core::core::dto::DataQueryAction::kInsightsChart;
    chart_empty_request.lookback_days = 7;
    const auto chart_empty_result =
        fixture.runtime.runtime_api->query().RunDataQuery(chart_empty_request);
    if (chart_empty_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart, empty) should fail "
                   "when the database does not exist.\n";
    } else if (chart_empty_result.error_message.empty()) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart, empty) should return "
                   "a non-empty error message.\n";
    }
    if (std::filesystem::exists(fixture.paths.db_path)) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart, empty) should not "
                   "create a database file.\n";
    }

    tracer_core::core::dto::DataQueryRequest chart_invalid_request;
    chart_invalid_request.action =
        tracer_core::core::dto::DataQueryAction::kInsightsChart;
    chart_invalid_request.lookback_days = 0;
    const auto chart_invalid_result =
        fixture.runtime.runtime_api->query().RunDataQuery(
            chart_invalid_request);
    if (chart_invalid_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart) should reject "
                   "lookback_days <= 0.\n";
    } else if (!Contains(chart_invalid_result.error_message,
                         "--lookback-days")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart) invalid lookback error "
                   "should mention --lookback-days.\n";
    }

    tracer_core::core::dto::DataQueryRequest chart_missing_range_request;
    chart_missing_range_request.action =
        tracer_core::core::dto::DataQueryAction::kInsightsChart;
    chart_missing_range_request.from_date = "2026-02-01";
    const auto chart_missing_range_result =
        fixture.runtime.runtime_api->query().RunDataQuery(
            chart_missing_range_request);
    if (chart_missing_range_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart) should reject missing "
                   "to_date when from_date is provided.\n";
    }

    tracer_core::core::dto::DataQueryRequest chart_invalid_range_order_request;
    chart_invalid_range_order_request.action =
        tracer_core::core::dto::DataQueryAction::kInsightsChart;
    chart_invalid_range_order_request.from_date = "2026-02-15";
    chart_invalid_range_order_request.to_date = "2026-02-01";
    const auto chart_invalid_range_order_result =
        fixture.runtime.runtime_api->query().RunDataQuery(
            chart_invalid_range_order_request);
    if (chart_invalid_range_order_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(insights-chart) should reject "
                   "from_date > to_date.\n";
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Android runtime config smoke test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Android runtime config smoke test threw non-standard "
                 "exception.\n";
  }

  CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests::smoke

// infrastructure/tests/android_runtime/android_runtime_smoke_config_tests.cpp
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

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
    tracer_core::core::dto::DataQueryRequest chart_empty_request;
    chart_empty_request.action =
        tracer_core::core::dto::DataQueryAction::kReportChart;
    chart_empty_request.lookback_days = 7;
    if (const auto chart_empty_result = RunDataQueryOrRecordFailure(
            fixture.runtime.core_api, chart_empty_request,
            "RunDataQuery(report-chart, empty)", failures);
        chart_empty_result.has_value()) {
      if (const auto payload = ParseJsonOrRecordFailure(
              chart_empty_result->content, "report-chart empty", failures);
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
    }

    tracer_core::core::dto::DataQueryRequest chart_invalid_request;
    chart_invalid_request.action =
        tracer_core::core::dto::DataQueryAction::kReportChart;
    chart_invalid_request.lookback_days = 0;
    const auto chart_invalid_result =
        fixture.runtime.core_api->RunDataQuery(chart_invalid_request);
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

    tracer_core::core::dto::DataQueryRequest chart_missing_range_request;
    chart_missing_range_request.action =
        tracer_core::core::dto::DataQueryAction::kReportChart;
    chart_missing_range_request.from_date = "2026-02-01";
    const auto chart_missing_range_result =
        fixture.runtime.core_api->RunDataQuery(chart_missing_range_request);
    if (chart_missing_range_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart) should reject missing "
                   "to_date when from_date is provided.\n";
    }

    tracer_core::core::dto::DataQueryRequest chart_invalid_range_order_request;
    chart_invalid_range_order_request.action =
        tracer_core::core::dto::DataQueryAction::kReportChart;
    chart_invalid_range_order_request.from_date = "2026-02-15";
    chart_invalid_range_order_request.to_date = "2026-02-01";
    const auto chart_invalid_range_order_result =
        fixture.runtime.core_api->RunDataQuery(
            chart_invalid_range_order_request);
    if (chart_invalid_range_order_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(report-chart) should reject "
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

// infrastructure/tests/android_runtime/android_runtime_smoke_io_tests.cpp
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

namespace android_runtime_tests::smoke {
namespace {

struct ChartProbeContext {
  std::optional<std::string> latest_date;
  std::optional<std::string> earliest_date;
  std::optional<std::string> selected_root;
};

auto ProbeChartRange(const std::shared_ptr<ITracerCoreApi>& core_api,
                     ChartProbeContext& chart_probe, int& failures) -> void {
  tracer_core::core::dto::DataQueryRequest chart_range_request;
  chart_range_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_range_request.lookback_days = 7;
  if (const auto chart_range_result = RunDataQueryOrRecordFailure(
          core_api, chart_range_request, "RunDataQuery(report-chart, range)",
          failures);
      chart_range_result.has_value()) {
    if (const auto payload = ParseJsonOrRecordFailure(
            chart_range_result->content, "report-chart range", failures);
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
          chart_probe.selected_root = roots.front().get<std::string>();
        }
        for (size_t index = 1; index < roots.size(); ++index) {
          if (roots[index - 1].is_string() && roots[index].is_string() &&
              roots[index].get<std::string>() <
                  roots[index - 1].get<std::string>()) {
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
          chart_probe.earliest_date = first["date"].get<std::string>();
        }
        const auto& last = series_it->back();
        if (last.is_object() && last.contains("date") &&
            last["date"].is_string()) {
          chart_probe.latest_date = last["date"].get<std::string>();
        }
      }
    }
  }
}

auto VerifyExplicitChartRange(const std::shared_ptr<ITracerCoreApi>& core_api,
                              const ChartProbeContext& chart_probe,
                              int& failures) -> void {
  if (!chart_probe.earliest_date.has_value() ||
      !chart_probe.latest_date.has_value()) {
    return;
  }

  tracer_core::core::dto::DataQueryRequest chart_explicit_range_request;
  chart_explicit_range_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_explicit_range_request.from_date = *chart_probe.earliest_date;
  chart_explicit_range_request.to_date = *chart_probe.latest_date;
  if (const auto chart_explicit_range_result = RunDataQueryOrRecordFailure(
          core_api, chart_explicit_range_request,
          "RunDataQuery(report-chart, explicit range)", failures);
      chart_explicit_range_result.has_value()) {
    if (const auto payload =
            ParseJsonOrRecordFailure(chart_explicit_range_result->content,
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
      if (payload->value("from_date", std::string()) !=
              *chart_probe.earliest_date ||
          payload->value("to_date", std::string()) !=
              *chart_probe.latest_date) {
        ++failures;
        std::cerr << "[FAIL] report-chart explicit range should echo "
                     "resolved from_date/to_date.\n";
      }
    }
  }
}

auto VerifyStatsForRootScenarios(
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const ChartProbeContext& chart_probe, std::string_view missing_root,
    int& failures) -> void {
  if (!chart_probe.latest_date.has_value()) {
    return;
  }

  tracer_core::core::dto::DataQueryRequest stats_base_request;
  stats_base_request.action =
      tracer_core::core::dto::DataQueryAction::kDaysStats;
  stats_base_request.tree_period = "day";
  stats_base_request.tree_period_argument = *chart_probe.latest_date;

  const auto stats_base_result = core_api->RunDataQuery(stats_base_request);
  if (!stats_base_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunDataQuery(days-stats, baseline day) should "
                 "succeed: "
              << stats_base_result.error_message << '\n';
  } else if (!Contains(stats_base_result.content, "## Day Duration Stats")) {
    ++failures;
    std::cerr << "[FAIL] days-stats baseline output should include "
                 "'## Day Duration Stats'.\n";
  }

  tracer_core::core::dto::DataQueryRequest stats_empty_root_request =
      stats_base_request;
  stats_empty_root_request.root = "   ";
  const auto stats_empty_root_result =
      core_api->RunDataQuery(stats_empty_root_request);
  if (!stats_empty_root_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunDataQuery(days-stats, empty root) should "
                 "succeed: "
              << stats_empty_root_result.error_message << '\n';
  } else if (stats_base_result.ok &&
             stats_empty_root_result.content != stats_base_result.content) {
    ++failures;
    std::cerr << "[FAIL] days-stats empty root should behave like "
                 "no root filter.\n";
  }

  if (chart_probe.selected_root.has_value()) {
    tracer_core::core::dto::DataQueryRequest stats_valid_root_request =
        stats_base_request;
    stats_valid_root_request.root = *chart_probe.selected_root;
    const auto stats_valid_root_result =
        core_api->RunDataQuery(stats_valid_root_request);
    if (!stats_valid_root_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats, selected root) should "
                   "succeed: "
                << stats_valid_root_result.error_message << '\n';
    } else if (!Contains(stats_valid_root_result.content,
                         "## Day Duration Stats")) {
      ++failures;
      std::cerr << "[FAIL] days-stats selected root output should "
                   "include '## Day Duration Stats'.\n";
    }
  }

  tracer_core::core::dto::DataQueryRequest stats_missing_root_request =
      stats_base_request;
  stats_missing_root_request.root = std::string(missing_root);
  const auto stats_missing_root_result =
      core_api->RunDataQuery(stats_missing_root_request);
  if (!stats_missing_root_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunDataQuery(days-stats, missing root) should "
                 "succeed: "
              << stats_missing_root_result.error_message << '\n';
  } else if (!Contains(stats_missing_root_result.content, "**Days**: 0")) {
    ++failures;
    std::cerr << "[FAIL] days-stats missing root output should "
                 "include '**Days**: 0'.\n";
  }
}

auto VerifyChartForRootScenarios(
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const ChartProbeContext& chart_probe, int& failures) -> void {
  if (!chart_probe.selected_root.has_value()) {
    return;
  }

  const std::string missing_root = "nosuchroot";

  tracer_core::core::dto::DataQueryRequest chart_root_request;
  chart_root_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_root_request.lookback_days = 7;
  chart_root_request.root = *chart_probe.selected_root;
  if (const auto chart_root_result = RunDataQueryOrRecordFailure(
          core_api, chart_root_request,
          "RunDataQuery(report-chart, selected root)", failures);
      chart_root_result.has_value()) {
    if (const auto payload = ParseJsonOrRecordFailure(
            chart_root_result->content, "report-chart selected root", failures);
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
          *chart_probe.selected_root) {
        ++failures;
        std::cerr << "[FAIL] report-chart selected_root should match "
                     "request --root value.\n";
      }
    }
  }

  tracer_core::core::dto::DataQueryRequest chart_root_overrides_project_request;
  chart_root_overrides_project_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_root_overrides_project_request.lookback_days = 7;
  chart_root_overrides_project_request.root = *chart_probe.selected_root;
  chart_root_overrides_project_request.project = "zzzz_override_project";
  if (const auto result = RunDataQueryOrRecordFailure(
          core_api, chart_root_overrides_project_request,
          "RunDataQuery(report-chart, root overrides project)", failures);
      result.has_value()) {
    if (const auto payload = ParseJsonOrRecordFailure(
            result->content, "report-chart root overrides project", failures);
        payload.has_value()) {
      ValidateChartSeriesPayload(
          *payload, "report-chart root overrides project", failures);
      if (payload->value("selected_root", std::string()) !=
          *chart_probe.selected_root) {
        ++failures;
        std::cerr << "[FAIL] report-chart should prefer request --root over "
                     "--project when both are provided.\n";
      }
    }
  }

  tracer_core::core::dto::DataQueryRequest chart_missing_root_request;
  chart_missing_root_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_missing_root_request.lookback_days = 7;
  chart_missing_root_request.root = missing_root;
  if (const auto result = RunDataQueryOrRecordFailure(
          core_api, chart_missing_root_request,
          "RunDataQuery(report-chart, missing root)", failures);
      result.has_value()) {
    if (const auto payload = ParseJsonOrRecordFailure(
            result->content, "report-chart missing root", failures);
        payload.has_value()) {
      ValidateChartSeriesPayload(*payload, "report-chart missing root",
                                 failures);
      if (payload->value("selected_root", std::string()) != missing_root) {
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
  }

  tracer_core::core::dto::DataQueryRequest chart_empty_root_request;
  chart_empty_root_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_empty_root_request.lookback_days = 7;
  chart_empty_root_request.root = "   ";
  if (const auto result = RunDataQueryOrRecordFailure(
          core_api, chart_empty_root_request,
          "RunDataQuery(report-chart, empty root)", failures);
      result.has_value()) {
    if (const auto payload = ParseJsonOrRecordFailure(
            result->content, "report-chart empty root", failures);
        payload.has_value()) {
      ValidateChartSeriesPayload(*payload, "report-chart empty root", failures);
      if (!payload->value("selected_root", std::string()).empty()) {
        ++failures;
        std::cerr << "[FAIL] report-chart empty root should normalize to "
                     "empty selected_root.\n";
      }
    }
  }

  tracer_core::core::dto::DataQueryRequest chart_boundary_request;
  chart_boundary_request.action =
      tracer_core::core::dto::DataQueryAction::kReportChart;
  chart_boundary_request.root = *chart_probe.selected_root;
  chart_boundary_request.lookback_days = 1;
  if (const auto result = RunDataQueryOrRecordFailure(
          core_api, chart_boundary_request,
          "RunDataQuery(report-chart, boundary)", failures);
      result.has_value()) {
    if (const auto payload = ParseJsonOrRecordFailure(
            result->content, "report-chart boundary", failures);
        payload.has_value()) {
      ValidateChartSeriesPayload(*payload, "report-chart boundary", failures);
      const auto series_it = payload->find("series");
      if (series_it == payload->end() || !series_it->is_array() ||
          series_it->size() != 1U) {
        ++failures;
        std::cerr << "[FAIL] report-chart boundary should return exactly 1 "
                     "point for lookback_days=1.\n";
      }
      if (chart_probe.latest_date.has_value() && series_it != payload->end() &&
          series_it->is_array() && !series_it->empty()) {
        const auto& first = series_it->front();
        if (first.is_object() && first.contains("date") &&
            first["date"].is_string() &&
            first["date"].get<std::string>() != *chart_probe.latest_date) {
          ++failures;
          std::cerr << "[FAIL] report-chart boundary point date should be "
                       "the latest date.\n";
        }
      }
    }
  }

  VerifyStatsForRootScenarios(core_api, chart_probe, missing_root, failures);
}

auto VerifyReportOutputs(const std::shared_ptr<ITracerCoreApi>& core_api,
                         int& failures) -> void {
  const auto report_result = RunAndCheckReportQuery(
      core_api,
      {.type = tracer_core::core::dto::ReportQueryType::kRecent,
       .argument = "1",
       .format = ReportFormat::kMarkdown},
      "markdown", failures);
  if (report_result && report_result->content.empty()) {
    ++failures;
    std::cerr << "[FAIL] RunReportQuery(markdown) should return non-empty "
                 "content.\n";
  }

  const auto day_report_result = RunAndCheckReportQuery(
      core_api,
      {.type = tracer_core::core::dto::ReportQueryType::kDay,
       .argument = "2026-02-01",
       .format = ReportFormat::kMarkdown},
      "day markdown", failures);
  if (day_report_result) {
    if (!Contains(day_report_result->content, "- **Date**: ")) {
      ++failures;
      std::cerr << "[FAIL] Android day markdown report should include "
                   "'Date' label.\n";
    }
    if (!Contains(day_report_result->content, "- **Total Time Recorded**: ")) {
      ++failures;
      std::cerr << "[FAIL] Android day markdown report should include "
                   "'Total Time Recorded' label.\n";
    }
  }

  const auto structured_result = core_api->RunStructuredReportQuery(
      {.type = tracer_core::core::dto::ReportQueryType::kRecent,
       .argument = "1"});
  if (!structured_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunStructuredReportQuery should succeed: "
              << structured_result.error_message << '\n';
  }
  if (structured_result.kind !=
      tracer_core::core::dto::StructuredReportKind::kRecent) {
    ++failures;
    std::cerr << "[FAIL] RunStructuredReportQuery should return kRecent.\n";
  }
}

}  // namespace

auto RunIoSmokeSection(int& failures) -> void {
  auto fixture_opt = BuildRuntimeFixture(
      "time_tracer_android_runtime_smoke_io_test", failures);
  if (!fixture_opt.has_value()) {
    return;
  }

  RuntimeFixture fixture = std::move(*fixture_opt);
  try {
    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = fixture.input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        fixture.runtime.core_api->RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunIngest should succeed for report-chart test: "
                << ingest_result.error_message << '\n';
    } else {
      ChartProbeContext chart_probe;
      ProbeChartRange(fixture.runtime.core_api, chart_probe, failures);
      VerifyExplicitChartRange(fixture.runtime.core_api, chart_probe, failures);
      VerifyChartForRootScenarios(fixture.runtime.core_api, chart_probe,
                                  failures);
    }

    VerifyReportOutputs(fixture.runtime.core_api, failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Android runtime IO smoke test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Android runtime IO smoke test threw non-standard "
                 "exception.\n";
  }

  CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests::smoke

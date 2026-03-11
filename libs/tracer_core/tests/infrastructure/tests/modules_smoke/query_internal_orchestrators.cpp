import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.query.data.internal;
import tracer.core.infrastructure.query.data.orchestrators;
import tracer.core.infrastructure.query.data.repository;

#include "application/dto/core_requests.hpp"
#include "infrastructure/tests/modules_smoke/support.hpp"

auto RunInfrastructureModuleQueryInternalOrchestratorsSmoke() -> int {
  const auto kTrimCopy =
      &tracer::core::infrastructure::query::data::internal::TrimCopy;
  const auto kBuildCliFilters =
      &tracer::core::infrastructure::query::data::internal::BuildCliFilters;
  const auto kApplyTreePeriod =
      &tracer::core::infrastructure::query::data::internal::ApplyTreePeriod;
  const auto kValidateReportChartRequest =
      &tracer::core::infrastructure::query::data::internal::
          ValidateReportChartRequest;
  const auto kBuildReportChartContent =
      &tracer::core::infrastructure::query::data::internal::
          BuildReportChartContent;
  const auto kResolveExplicitDateRange =
      &tracer::core::infrastructure::query::data::orchestrators::
          ResolveExplicitDateRange;
  const auto kResolveRollingDateRange =
      &tracer::core::infrastructure::query::data::orchestrators::
          ResolveRollingDateRange;
  const auto kHandleYearsQuery =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleYearsQuery;
  const auto kHandleMonthsQuery =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleMonthsQuery;
  const auto kHandleDaysQuery =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleDaysQuery;
  const auto kHandleDaysStatsQuery =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleDaysStatsQuery;
  const auto kHandleReportChartQuery =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleReportChartQuery;
  const auto kHandleTreeQuery =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleTreeQuery;
  const auto kBoundaryReady =
      &tracer::core::infrastructure::query::data::orchestrators::BoundaryReady;
  (void)kTrimCopy;
  (void)kBuildCliFilters;
  (void)kApplyTreePeriod;
  (void)kValidateReportChartRequest;
  (void)kBuildReportChartContent;
  (void)kResolveExplicitDateRange;
  (void)kResolveRollingDateRange;
  (void)kHandleYearsQuery;
  (void)kHandleMonthsQuery;
  (void)kHandleDaysQuery;
  (void)kHandleDaysStatsQuery;
  (void)kHandleReportChartQuery;
  (void)kHandleTreeQuery;
  (void)kBoundaryReady;

  if (tracer::core::infrastructure::query::data::internal::TrimCopy(
          " 2026-02-01 ") != "2026-02-01") {
    return 100;
  }

  tracer_core::core::dto::DataQueryRequest filters_request{};
  filters_request.root = " Work_Sub ";
  filters_request.project = "Ignored";
  auto filters =
      tracer::core::infrastructure::query::data::internal::BuildCliFilters(
          filters_request);
  if (!filters.root.has_value() || *filters.root != "Work" ||
      filters.project.has_value()) {
    return 101;
  }

  tracer_core::core::dto::DataQueryRequest period_request{};
  period_request.tree_period = "month";
  period_request.tree_period_argument = "202602";
  tracer::core::infrastructure::query::data::internal::ApplyTreePeriod(
      period_request, nullptr, filters);
  if (!filters.from_date.has_value() || *filters.from_date != "2026-02-01" ||
      !filters.to_date.has_value() || *filters.to_date != "2026-02-28") {
    return 102;
  }

  tracer_core::core::dto::DataQueryRequest chart_request{};
  chart_request.lookback_days = 7;
  chart_request.from_date = "2026-02-01";
  chart_request.to_date = "2026-02-02";
  try {
    tracer::core::infrastructure::query::data::internal::
        ValidateReportChartRequest(chart_request);
  } catch (...) {
    return 103;
  }

  const auto explicit_errors =
      tracer::core::infrastructure::query::data::orchestrators::
          ExplicitDateRangeErrors{
              .missing_boundary_error = "missing",
              .validation =
                  {
                      .invalid_range_error = "range",
                      .invalid_date_error = "date",
                  },
          };
  const auto explicit_range =
      tracer::core::infrastructure::query::data::orchestrators::
          ResolveExplicitDateRange("2026-02-01", "2026-02-07",
                                   explicit_errors);
  if (!explicit_range.has_value() ||
      explicit_range->start_date != "2026-02-01" ||
      explicit_range->end_date != "2026-02-07") {
    return 104;
  }

  const auto rolling_range =
      tracer::core::infrastructure::query::data::orchestrators::
          ResolveRollingDateRange(7);
  if (rolling_range.start_date.size() != 10U ||
      rolling_range.end_date.size() != 10U ||
      !tracer::core::infrastructure::query::data::orchestrators::
          BoundaryReady()) {
    return 105;
  }

  std::error_code cleanup_error;
  const std::filesystem::path kQuerySmokeDir =
      std::filesystem::path("temp") / "phase15_infra_module_smoke";
  const std::filesystem::path kDbPath = kQuerySmokeDir / "query.sqlite";
  std::filesystem::remove_all(kQuerySmokeDir, cleanup_error);
  std::filesystem::create_directories(kQuerySmokeDir);

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    const auto chart_content =
        tracer::core::infrastructure::query::data::internal::
            BuildReportChartContent(connection.GetDb(), chart_request);
    if (chart_content.find("\"series\":[]") == std::string::npos) {
      return 103;
    }

    tracer::core::infrastructure::query::data::QueryFilters base_filters{};

    const auto years_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleYearsQuery(
                connection.GetDb(),
                tracer_core::core::dto::DataQueryOutputMode::kText);
    if (!years_output.ok ||
        years_output.content.find("Total: 0") == std::string::npos) {
      return 16;
    }

    const auto months_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleMonthsQuery(
                connection.GetDb(), base_filters,
                tracer_core::core::dto::DataQueryOutputMode::kText);
    if (!months_output.ok ||
        months_output.content.find("Total: 0") == std::string::npos) {
      return 17;
    }

    const auto days_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleDaysQuery(
                connection.GetDb(), base_filters,
                tracer_core::core::dto::DataQueryOutputMode::kText);
    if (!days_output.ok ||
        days_output.content.find("Total: 0") == std::string::npos) {
      return 18;
    }

    tracer_core::core::dto::DataQueryRequest stats_request{};
    const auto days_stats_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleDaysStatsQuery(
                connection.GetDb(), stats_request, base_filters,
                tracer_core::core::dto::DataQueryOutputMode::kText);
    if (!days_stats_output.ok ||
        days_stats_output.content.find("## Day Duration Stats") ==
            std::string::npos) {
      return 19;
    }

    const auto report_chart_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleReportChartQuery(
                connection.GetDb(), chart_request,
                tracer_core::core::dto::DataQueryOutputMode::kText);
    if (!report_chart_output.ok ||
        report_chart_output.content.find("\"series\":[]") ==
            std::string::npos) {
      return 20;
    }

    tracer_core::core::dto::DataQueryRequest tree_request{};
    const auto tree_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleTreeQuery(
                connection.GetDb(), tree_request, base_filters,
                tracer_core::core::dto::DataQueryOutputMode::kText);
    if (!tree_output.ok ||
        tree_output.content.find("Total: 0") == std::string::npos) {
      return 21;
    }
  } catch (...) {
    return 16;
  }

  std::filesystem::remove_all(kQuerySmokeDir, cleanup_error);
  return 0;
}

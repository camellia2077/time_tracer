import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.query.data.renderers;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.stats;

#include "application/dto/core_requests.hpp"
#include "infrastructure/tests/modules_smoke/support.hpp"

auto RunInfrastructureModuleQueryStatsRepositorySmoke() -> int {
  const auto kComputeDayDurationStats =
      &tracer::core::infrastructure::query::data::stats::
          ComputeDayDurationStats;
  const auto kBuildReportChartSeries =
      &tracer::core::infrastructure::query::data::stats::
          BuildReportChartSeries;
  const auto kStatsBoundaryReady =
      &tracer::core::infrastructure::query::data::stats::BoundaryReady;
  const auto kQueryYears =
      &tracer::core::infrastructure::query::data::QueryYears;
  const auto kQueryProjectTree =
      &tracer::core::infrastructure::query::data::QueryProjectTree;
  const auto kRenderListOutput =
      &tracer::core::infrastructure::query::data::renderers::RenderListOutput;
  const auto kRenderDayDurationsOutput =
      &tracer::core::infrastructure::query::data::renderers::
          RenderDayDurationsOutput;
  const auto kRenderersBoundaryReady =
      &tracer::core::infrastructure::query::data::renderers::BoundaryReady;
  (void)kComputeDayDurationStats;
  (void)kBuildReportChartSeries;
  (void)kStatsBoundaryReady;
  (void)kQueryYears;
  (void)kQueryProjectTree;
  (void)kRenderListOutput;
  (void)kRenderDayDurationsOutput;
  (void)kRenderersBoundaryReady;

  if (!tracer::core::infrastructure::query::data::stats::BoundaryReady()) {
    return 5;
  }
  if (!tracer::core::infrastructure::query::data::renderers::BoundaryReady()) {
    return 6;
  }

  const std::vector<tracer::core::infrastructure::query::data::DayDurationRow>
      kRows = {
          {.date = "2026-02-01", .total_seconds = 3600},
          {.date = "2026-02-03", .total_seconds = 7200},
      };
  const auto kStats =
      tracer::core::infrastructure::query::data::stats::
          ComputeDayDurationStats(kRows);
  if (kStats.count != 2 || kStats.min_seconds != 3600.0 ||
      kStats.max_seconds != 7200.0) {
    return 7;
  }

  const auto kSeries =
      tracer::core::infrastructure::query::data::stats::BuildReportChartSeries(
          {.start_date = "2026-02-01", .end_date = "2026-02-03"}, kRows);
  if (kSeries.series.size() != 3U ||
      kSeries.series.front().date != "2026-02-01" ||
      kSeries.series.back().date != "2026-02-03" ||
      kSeries.stats.active_days != 2 ||
      kSeries.stats.total_duration_seconds != 10800) {
    return 8;
  }

  const auto kSemanticOutput =
      tracer::core::infrastructure::query::data::renderers::RenderListOutput(
          "years", {"2026"},
          tracer_core::core::dto::DataQueryOutputMode::kSemanticJson);
  if (kSemanticOutput.find("\"output_mode\":\"semantic_json\"") ==
      std::string::npos) {
    return 9;
  }

  const auto kTextOutput =
      tracer::core::infrastructure::query::data::renderers::
          RenderDayDurationsOutput(
              "days_duration", kRows,
              tracer_core::core::dto::DataQueryOutputMode::kText);
  if (kTextOutput.find("2026-02-01") == std::string::npos ||
      kTextOutput.find("Total: 2") == std::string::npos) {
    return 10;
  }

  std::error_code cleanup_error;
  const std::filesystem::path kQuerySmokeDir =
      std::filesystem::path("temp") / "phase13_infra_module_smoke";
  const std::filesystem::path kDbPath = kQuerySmokeDir / "query.sqlite";
  std::filesystem::remove_all(kQuerySmokeDir, cleanup_error);
  std::filesystem::create_directories(kQuerySmokeDir);

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    tracer::core::infrastructure::query::data::QueryFilters filters{};

    if (tracer::core::infrastructure::query::data::QueryLatestTrackedDate(
            connection.GetDb())
            .has_value()) {
      return 11;
    }
    if (!tracer::core::infrastructure::query::data::QueryProjectRootNames(
             connection.GetDb())
             .empty()) {
      return 12;
    }
    if (!tracer::core::infrastructure::query::data::QueryDayDurations(
             connection.GetDb(), filters)
             .empty()) {
      return 13;
    }
    if (!tracer::core::infrastructure::query::data::QueryDays(
             connection.GetDb(), filters.kYear, filters.kMonth,
             filters.from_date, filters.to_date, filters.reverse,
             filters.limit)
             .empty()) {
      return 14;
    }
    if (!tracer::core::infrastructure::query::data::QueryProjectTree(
             connection.GetDb(), filters)
             .empty() ||
        !tracer::core::infrastructure::query::data::QueryYears(connection.GetDb())
             .empty()) {
      return 15;
    }
  } catch (...) {
    return 11;
  }

  std::filesystem::remove_all(kQuerySmokeDir, cleanup_error);
  return 0;
}

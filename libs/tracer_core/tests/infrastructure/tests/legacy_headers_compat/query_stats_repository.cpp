#include "infrastructure/tests/legacy_headers_compat/support.hpp"

auto TestLegacyQueryDataStatsHeaders(int& failures) -> void {
  using LegacyReportChartSeriesPoint =
      tracer_core::infrastructure::query::data::stats::ReportChartSeriesPoint;
  using LegacyReportChartSeriesResult =
      tracer_core::infrastructure::query::data::stats::ReportChartSeriesResult;
  using CanonicalReportChartSeriesPoint =
      tracer::core::infrastructure::query::data::stats::ReportChartSeriesPoint;
  using CanonicalReportChartSeriesResult =
      tracer::core::infrastructure::query::data::stats::ReportChartSeriesResult;

  Expect(std::is_class_v<LegacyReportChartSeriesPoint>,
         "Legacy ReportChartSeriesPoint header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyReportChartSeriesResult>,
         "Legacy ReportChartSeriesResult header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalReportChartSeriesPoint>,
         "Canonical ReportChartSeriesPoint header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalReportChartSeriesResult>,
         "Canonical ReportChartSeriesResult header contract should be visible.",
         failures);

  using LegacyComputeDayDurationStatsSignature =
      decltype(
          tracer_core::infrastructure::query::data::stats::
              ComputeDayDurationStats);
  using CanonicalComputeDayDurationStatsSignature =
      decltype(
          tracer::core::infrastructure::query::data::stats::
              ComputeDayDurationStats);
  using LegacyBuildReportChartSeriesSignature =
      decltype(
          tracer_core::infrastructure::query::data::stats::
              BuildReportChartSeries);
  using CanonicalBuildReportChartSeriesSignature =
      decltype(
          tracer::core::infrastructure::query::data::stats::
              BuildReportChartSeries);
  using LegacyBoundaryReadySignature =
      decltype(tracer_core::infrastructure::query::data::stats::BoundaryReady);
  using CanonicalBoundaryReadySignature =
      decltype(tracer::core::infrastructure::query::data::stats::BoundaryReady);

  Expect(std::is_function_v<LegacyComputeDayDurationStatsSignature>,
         "Legacy ComputeDayDurationStats declaration should remain visible.",
         failures);
  Expect(std::is_function_v<CanonicalComputeDayDurationStatsSignature>,
         "Canonical ComputeDayDurationStats declaration should be visible.",
         failures);
  Expect((std::is_same_v<LegacyComputeDayDurationStatsSignature,
                         CanonicalComputeDayDurationStatsSignature>),
         "Legacy/canonical ComputeDayDurationStats signatures should match.",
         failures);
  Expect(std::is_function_v<LegacyBuildReportChartSeriesSignature>,
         "Legacy BuildReportChartSeries declaration should remain visible.",
         failures);
  Expect(std::is_function_v<CanonicalBuildReportChartSeriesSignature>,
         "Canonical BuildReportChartSeries declaration should be visible.",
         failures);
  Expect((std::is_same_v<LegacyBuildReportChartSeriesSignature,
                         CanonicalBuildReportChartSeriesSignature>),
         "Legacy/canonical BuildReportChartSeries signatures should match.",
         failures);
  Expect(std::is_function_v<LegacyBoundaryReadySignature>,
         "Legacy BoundaryReady declaration should remain visible.", failures);
  Expect(std::is_function_v<CanonicalBoundaryReadySignature>,
         "Canonical BoundaryReady declaration should be visible.", failures);
  Expect((std::is_same_v<LegacyBoundaryReadySignature,
                         CanonicalBoundaryReadySignature>),
         "Legacy/canonical BoundaryReady signatures should match.", failures);
}

auto TestLegacyQueryDataRepositoryAndRendererHeaders(int& failures) -> void {
  using CanonicalDataQueryAction =
      tracer::core::infrastructure::query::data::DataQueryAction;
  using CanonicalDayDurationRow =
      tracer::core::infrastructure::query::data::DayDurationRow;
  using CanonicalQueryFilters =
      tracer::core::infrastructure::query::data::QueryFilters;
  using LegacyDataQueryAction =
      tracer_core::infrastructure::query::data::DataQueryAction;
  using LegacyDayDurationRow =
      tracer_core::infrastructure::query::data::DayDurationRow;
  using LegacyQueryFilters =
      tracer_core::infrastructure::query::data::QueryFilters;

  Expect(std::is_enum_v<LegacyDataQueryAction>,
         "Legacy DataQueryAction header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyDayDurationRow>,
         "Legacy DayDurationRow header path should remain visible.", failures);
  Expect(std::is_class_v<LegacyQueryFilters>,
         "Legacy QueryFilters header path should remain visible.", failures);
  Expect(std::is_enum_v<CanonicalDataQueryAction>,
         "Canonical DataQueryAction header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalDayDurationRow>,
         "Canonical DayDurationRow header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalQueryFilters>,
         "Canonical QueryFilters header contract should be visible.",
         failures);

  const auto legacy_query_years =
      &tracer_core::infrastructure::query::data::QueryYears;
  const auto canonical_query_years =
      &tracer::core::infrastructure::query::data::QueryYears;
  const auto legacy_query_project_tree =
      &tracer_core::infrastructure::query::data::QueryProjectTree;
  const auto canonical_query_project_tree =
      &tracer::core::infrastructure::query::data::QueryProjectTree;
  const auto legacy_render_list_output =
      &tracer_core::infrastructure::query::data::renderers::RenderListOutput;
  const auto canonical_render_list_output =
      &tracer::core::infrastructure::query::data::renderers::RenderListOutput;
  const auto legacy_build_semantic_list =
      &tracer_core::infrastructure::query::data::renderers::
          BuildSemanticListPayload;
  const auto canonical_build_semantic_list =
      &tracer::core::infrastructure::query::data::renderers::
          BuildSemanticListPayload;
  const auto legacy_renderers_boundary =
      &tracer_core::infrastructure::query::data::renderers::BoundaryReady;
  const auto canonical_renderers_boundary =
      &tracer::core::infrastructure::query::data::renderers::BoundaryReady;
  (void)legacy_query_years;
  (void)canonical_query_years;
  (void)legacy_query_project_tree;
  (void)canonical_query_project_tree;
  (void)legacy_render_list_output;
  (void)canonical_render_list_output;
  (void)legacy_build_semantic_list;
  (void)canonical_build_semantic_list;
  (void)legacy_renderers_boundary;
  (void)canonical_renderers_boundary;

  const std::vector<std::string> year_labels = {"2026"};
  const auto legacy_text_output =
      tracer_core::infrastructure::query::data::renderers::RenderListOutput(
          "years", year_labels,
          tracer_core::core::dto::DataQueryOutputMode::kText);
  Expect(legacy_text_output.find("2026") != std::string::npos,
         "Legacy RenderListOutput should remain visible.", failures);

  const auto canonical_semantic_output =
      tracer::core::infrastructure::query::data::renderers::RenderListOutput(
          "years", year_labels,
          tracer_core::core::dto::DataQueryOutputMode::kSemanticJson);
  Expect(canonical_semantic_output.find("\"output_mode\":\"semantic_json\"") !=
             std::string::npos,
         "Canonical RenderListOutput should support semantic json.", failures);
  Expect(tracer_core::infrastructure::query::data::renderers::BoundaryReady(),
         "Legacy renderers boundary should remain visible.", failures);
  Expect(tracer::core::infrastructure::query::data::renderers::BoundaryReady(),
         "Canonical renderers boundary should remain visible.", failures);

  const std::filesystem::path legacy_root =
      std::filesystem::path("temp") / "phase13_legacy_query_data";
  const std::filesystem::path canonical_root =
      std::filesystem::path("temp") / "phase13_canonical_query_data";
  const std::filesystem::path legacy_db_path = legacy_root / "query.sqlite";
  const std::filesystem::path canonical_db_path =
      canonical_root / "query.sqlite";
  std::error_code cleanup_error;
  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);
  std::filesystem::create_directories(legacy_root, cleanup_error);
  std::filesystem::create_directories(canonical_root, cleanup_error);

  try {
    infrastructure::persistence::importer::sqlite::Connection legacy_connection(
        legacy_db_path.string());
    infrastructure::persistence::importer::sqlite::Connection
        canonical_connection(canonical_db_path.string());
    LegacyQueryFilters legacy_filters{};
    CanonicalQueryFilters canonical_filters{};

    Expect(!tracer_core::infrastructure::query::data::QueryLatestTrackedDate(
                legacy_connection.GetDb())
                .has_value(),
           "Legacy QueryLatestTrackedDate should be empty on a fresh db.",
           failures);
    Expect(tracer::core::infrastructure::query::data::QueryProjectRootNames(
               canonical_connection.GetDb())
               .empty(),
           "Canonical QueryProjectRootNames should be empty on a fresh db.",
           failures);
    Expect(tracer_core::infrastructure::query::data::QueryDayDurations(
               legacy_connection.GetDb(), legacy_filters)
               .empty(),
           "Legacy QueryDayDurations should be empty on a fresh db.",
           failures);
    Expect(tracer::core::infrastructure::query::data::QueryDays(
               canonical_connection.GetDb(), canonical_filters.kYear,
               canonical_filters.kMonth, canonical_filters.from_date,
               canonical_filters.to_date, canonical_filters.reverse,
               canonical_filters.limit)
               .empty(),
           "Canonical QueryDays should be empty on a fresh db.", failures);
  } catch (...) {
    Expect(false,
           "Query repository legacy/canonical adapters should construct and query successfully.",
           failures);
  }

  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);
}

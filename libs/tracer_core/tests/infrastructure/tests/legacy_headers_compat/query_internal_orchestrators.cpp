#include "infrastructure/tests/legacy_headers_compat/support.hpp"

auto TestLegacyQueryDataInternalHeaders(int& failures) -> void {
  const auto legacy_trim_copy =
      &infrastructure::persistence::data_query_service_internal::TrimCopy;
  const auto canonical_trim_copy =
      &tracer::core::infrastructure::query::data::internal::TrimCopy;
  const auto legacy_build_cli_filters =
      &infrastructure::persistence::data_query_service_internal::
          BuildCliFilters;
  const auto canonical_build_cli_filters =
      &tracer::core::infrastructure::query::data::internal::BuildCliFilters;
  const auto legacy_apply_tree_period =
      &infrastructure::persistence::data_query_service_internal::
          ApplyTreePeriod;
  const auto canonical_apply_tree_period =
      &tracer::core::infrastructure::query::data::internal::ApplyTreePeriod;
  const auto legacy_validate_report_chart_request =
      &infrastructure::persistence::data_query_service_internal::
          ValidateReportChartRequest;
  const auto canonical_validate_report_chart_request =
      &tracer::core::infrastructure::query::data::internal::
          ValidateReportChartRequest;
  const auto legacy_build_report_chart_content =
      &infrastructure::persistence::data_query_service_internal::
          BuildReportChartContent;
  const auto canonical_build_report_chart_content =
      &tracer::core::infrastructure::query::data::internal::
          BuildReportChartContent;
  (void)legacy_trim_copy;
  (void)canonical_trim_copy;
  (void)legacy_build_cli_filters;
  (void)canonical_build_cli_filters;
  (void)legacy_apply_tree_period;
  (void)canonical_apply_tree_period;
  (void)legacy_validate_report_chart_request;
  (void)canonical_validate_report_chart_request;
  (void)legacy_build_report_chart_content;
  (void)canonical_build_report_chart_content;

  Expect(infrastructure::persistence::data_query_service_internal::TrimCopy(
             " 2026-02-01 ") == "2026-02-01",
         "Legacy query.data.internal TrimCopy should remain visible.",
         failures);
  Expect(tracer::core::infrastructure::query::data::internal::TrimCopy(
             " 2026-02-01 ") == "2026-02-01",
         "Canonical query.data.internal TrimCopy should remain visible.",
         failures);

  tracer_core::core::dto::DataQueryRequest build_filters_request{};
  build_filters_request.root = " Work_Sub ";
  build_filters_request.project = "Ignored";
  const auto legacy_filters =
      infrastructure::persistence::data_query_service_internal::
          BuildCliFilters(build_filters_request);
  const auto canonical_filters =
      tracer::core::infrastructure::query::data::internal::BuildCliFilters(
          build_filters_request);
  Expect(legacy_filters.root.has_value() && *legacy_filters.root == "Work" &&
             !legacy_filters.project.has_value(),
         "Legacy BuildCliFilters should normalize root filters.", failures);
  Expect(canonical_filters.root.has_value() &&
             *canonical_filters.root == "Work" &&
             !canonical_filters.project.has_value(),
         "Canonical BuildCliFilters should normalize root filters.",
         failures);

  tracer_core::core::dto::DataQueryRequest period_request{};
  period_request.tree_period = "month";
  period_request.tree_period_argument = "202602";
  auto legacy_period_filters = legacy_filters;
  auto canonical_period_filters = canonical_filters;
  infrastructure::persistence::data_query_service_internal::ApplyTreePeriod(
      period_request, nullptr, legacy_period_filters);
  tracer::core::infrastructure::query::data::internal::ApplyTreePeriod(
      period_request, nullptr, canonical_period_filters);
  Expect(legacy_period_filters.from_date.has_value() &&
             *legacy_period_filters.from_date == "2026-02-01" &&
             legacy_period_filters.to_date.has_value() &&
             *legacy_period_filters.to_date == "2026-02-28",
         "Legacy ApplyTreePeriod should normalize month periods.",
         failures);
  Expect(canonical_period_filters.from_date.has_value() &&
             *canonical_period_filters.from_date == "2026-02-01" &&
             canonical_period_filters.to_date.has_value() &&
             *canonical_period_filters.to_date == "2026-02-28",
         "Canonical ApplyTreePeriod should normalize month periods.",
         failures);

  tracer_core::core::dto::DataQueryRequest chart_request{};
  chart_request.lookback_days = 7;
  chart_request.from_date = "2026-02-01";
  chart_request.to_date = "2026-02-02";
  infrastructure::persistence::data_query_service_internal::
      ValidateReportChartRequest(chart_request);
  tracer::core::infrastructure::query::data::internal::
      ValidateReportChartRequest(chart_request);

  const std::filesystem::path legacy_root =
      std::filesystem::path("temp") / "phase15_legacy_query_internal";
  const std::filesystem::path canonical_root =
      std::filesystem::path("temp") / "phase15_canonical_query_internal";
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

    const auto legacy_chart_content =
        infrastructure::persistence::data_query_service_internal::
            BuildReportChartContent(legacy_connection.GetDb(), chart_request);
    const auto canonical_chart_content =
        tracer::core::infrastructure::query::data::internal::
            BuildReportChartContent(canonical_connection.GetDb(),
                                    chart_request);
    Expect(legacy_chart_content.find("\"series\":[]") != std::string::npos,
           "Legacy BuildReportChartContent should remain visible.",
           failures);
    Expect(canonical_chart_content.find("\"series\":[]") != std::string::npos,
           "Canonical BuildReportChartContent should remain visible.",
           failures);
  } catch (...) {
    Expect(false,
           "Query data internal legacy/canonical adapters should construct and query successfully.",
           failures);
  }

  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);
}

auto TestLegacyQueryDataOrchestratorHeaders(int& failures) -> void {
  using CanonicalDateRangeBoundaries =
      tracer::core::infrastructure::query::data::orchestrators::
          DateRangeBoundaries;
  using CanonicalDateRangeValidationErrors =
      tracer::core::infrastructure::query::data::orchestrators::
          DateRangeValidationErrors;
  using CanonicalExplicitDateRangeErrors =
      tracer::core::infrastructure::query::data::orchestrators::
          ExplicitDateRangeErrors;
  using CanonicalResolvedDateRange =
      tracer::core::infrastructure::query::data::orchestrators::
          ResolvedDateRange;
  using LegacyDateRangeBoundaries =
      tracer_core::infrastructure::query::data::orchestrators::
          DateRangeBoundaries;
  using LegacyDateRangeValidationErrors =
      tracer_core::infrastructure::query::data::orchestrators::
          DateRangeValidationErrors;
  using LegacyExplicitDateRangeErrors =
      tracer_core::infrastructure::query::data::orchestrators::
          ExplicitDateRangeErrors;
  using LegacyResolvedDateRange =
      tracer_core::infrastructure::query::data::orchestrators::
          ResolvedDateRange;

  Expect(std::is_class_v<LegacyResolvedDateRange>,
         "Legacy ResolvedDateRange header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyDateRangeValidationErrors>,
         "Legacy DateRangeValidationErrors header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyDateRangeBoundaries>,
         "Legacy DateRangeBoundaries header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyExplicitDateRangeErrors>,
         "Legacy ExplicitDateRangeErrors header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalResolvedDateRange>,
         "Canonical ResolvedDateRange header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalDateRangeValidationErrors>,
         "Canonical DateRangeValidationErrors header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalDateRangeBoundaries>,
         "Canonical DateRangeBoundaries header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalExplicitDateRangeErrors>,
         "Canonical ExplicitDateRangeErrors header contract should be visible.",
         failures);

  const auto legacy_handle_years =
      &tracer_core::infrastructure::query::data::orchestrators::
          HandleYearsQuery;
  const auto canonical_handle_years =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleYearsQuery;
  const auto legacy_handle_days_stats =
      &tracer_core::infrastructure::query::data::orchestrators::
          HandleDaysStatsQuery;
  const auto canonical_handle_days_stats =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleDaysStatsQuery;
  const auto legacy_handle_report_chart =
      &tracer_core::infrastructure::query::data::orchestrators::
          HandleReportChartQuery;
  const auto canonical_handle_report_chart =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleReportChartQuery;
  const auto legacy_handle_tree =
      &tracer_core::infrastructure::query::data::orchestrators::
          HandleTreeQuery;
  const auto canonical_handle_tree =
      &tracer::core::infrastructure::query::data::orchestrators::
          HandleTreeQuery;
  const auto legacy_resolve_explicit =
      &tracer_core::infrastructure::query::data::orchestrators::
          ResolveExplicitDateRange;
  const auto canonical_resolve_explicit =
      &tracer::core::infrastructure::query::data::orchestrators::
          ResolveExplicitDateRange;
  const auto legacy_resolve_rolling =
      &tracer_core::infrastructure::query::data::orchestrators::
          ResolveRollingDateRange;
  const auto canonical_resolve_rolling =
      &tracer::core::infrastructure::query::data::orchestrators::
          ResolveRollingDateRange;
  const auto legacy_orchestrators_boundary =
      &tracer_core::infrastructure::query::data::orchestrators::
          BoundaryReady;
  const auto canonical_orchestrators_boundary =
      &tracer::core::infrastructure::query::data::orchestrators::
          BoundaryReady;
  (void)legacy_handle_years;
  (void)canonical_handle_years;
  (void)legacy_handle_days_stats;
  (void)canonical_handle_days_stats;
  (void)legacy_handle_report_chart;
  (void)canonical_handle_report_chart;
  (void)legacy_handle_tree;
  (void)canonical_handle_tree;
  (void)legacy_resolve_explicit;
  (void)canonical_resolve_explicit;
  (void)legacy_resolve_rolling;
  (void)canonical_resolve_rolling;
  (void)legacy_orchestrators_boundary;
  (void)canonical_orchestrators_boundary;

  const LegacyExplicitDateRangeErrors legacy_errors{
      .missing_boundary_error = "missing",
      .validation =
          {
              .invalid_range_error = "range",
              .invalid_date_error = "date",
          },
  };
  const CanonicalExplicitDateRangeErrors canonical_errors{
      .missing_boundary_error = "missing",
      .validation =
          {
              .invalid_range_error = "range",
              .invalid_date_error = "date",
          },
  };
  const std::optional<std::string> from_date = "2026-02-01";
  const std::optional<std::string> to_date = "2026-02-07";

  const auto legacy_range =
      tracer_core::infrastructure::query::data::orchestrators::
          ResolveExplicitDateRange(from_date, to_date, legacy_errors);
  const auto canonical_range =
      tracer::core::infrastructure::query::data::orchestrators::
          ResolveExplicitDateRange(from_date, to_date, canonical_errors);
  Expect(legacy_range.has_value() && legacy_range->end_date == "2026-02-07",
         "Legacy ResolveExplicitDateRange should remain visible.", failures);
  Expect(canonical_range.has_value() &&
             canonical_range->start_date == "2026-02-01",
         "Canonical ResolveExplicitDateRange should remain visible.",
         failures);

  const auto legacy_rolling =
      tracer_core::infrastructure::query::data::orchestrators::
          ResolveRollingDateRange(7);
  const auto canonical_rolling =
      tracer::core::infrastructure::query::data::orchestrators::
          ResolveRollingDateRange(7);
  Expect(legacy_rolling.start_date.size() == 10U &&
             legacy_rolling.end_date.size() == 10U,
         "Legacy ResolveRollingDateRange should produce ISO dates.",
         failures);
  Expect(canonical_rolling.start_date.size() == 10U &&
             canonical_rolling.end_date.size() == 10U,
         "Canonical ResolveRollingDateRange should produce ISO dates.",
         failures);
  Expect(
      tracer_core::infrastructure::query::data::orchestrators::BoundaryReady(),
      "Legacy orchestrators boundary should remain visible.", failures);
  Expect(tracer::core::infrastructure::query::data::orchestrators::
             BoundaryReady(),
         "Canonical orchestrators boundary should remain visible.", failures);

  const std::filesystem::path legacy_root =
      std::filesystem::path("temp") / "phase14_legacy_query_orchestrators";
  const std::filesystem::path canonical_root =
      std::filesystem::path("temp") / "phase14_canonical_query_orchestrators";
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

    const auto legacy_years_output =
        tracer_core::infrastructure::query::data::orchestrators::
            HandleYearsQuery(
                legacy_connection.GetDb(),
                tracer_core::core::dto::DataQueryOutputMode::kText);
    const auto canonical_years_output =
        tracer::core::infrastructure::query::data::orchestrators::
            HandleYearsQuery(
                canonical_connection.GetDb(),
                tracer_core::core::dto::DataQueryOutputMode::kText);
    Expect(legacy_years_output.ok &&
               legacy_years_output.content.find("Total: 0") !=
                   std::string::npos,
           "Legacy HandleYearsQuery should remain visible on a fresh db.",
           failures);
    Expect(canonical_years_output.ok &&
               canonical_years_output.content.find("Total: 0") !=
                   std::string::npos,
           "Canonical HandleYearsQuery should remain visible on a fresh db.",
           failures);
  } catch (...) {
    Expect(false,
           "Query orchestrators legacy/canonical adapters should construct and query successfully.",
           failures);
  }

  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);
}

#include "infrastructure/tests/legacy_headers_compat/support.hpp"

namespace {

auto BuildMinimalReportCatalog() -> ReportCatalog {
  ReportCatalog catalog;
  catalog.loaded_reports.markdown.day.labels.date_label = "Date";
  return catalog;
}

}  // namespace

auto TestLegacyReportsQueryingHeaders(int& failures) -> void {
  using CanonicalDailyReportService =
      tracer::core::infrastructure::reports::services::DailyReportService;
  using CanonicalLazySqliteReportQueryService =
      tracer::core::infrastructure::reports::LazySqliteReportQueryService;
  using CanonicalMonthlyReportService =
      tracer::core::infrastructure::reports::services::MonthlyReportService;
  using CanonicalReportService =
      tracer::core::infrastructure::reports::ReportService;
  using CanonicalWeeklyReportService =
      tracer::core::infrastructure::reports::services::WeeklyReportService;
  using CanonicalYearlyReportService =
      tracer::core::infrastructure::reports::services::YearlyReportService;
  using LegacyDailyReportService =
      infrastructure::reports::services::DailyReportService;
  using LegacyLazySqliteReportQueryService =
      infrastructure::reports::LazySqliteReportQueryService;
  using LegacyMonthlyReportService =
      infrastructure::reports::services::MonthlyReportService;
  using LegacyReportService = infrastructure::reports::ReportService;
  using LegacyWeeklyReportService =
      infrastructure::reports::services::WeeklyReportService;
  using LegacyYearlyReportService =
      infrastructure::reports::services::YearlyReportService;

  Expect(std::is_class_v<LegacyReportService>,
         "Legacy ReportService header path should remain visible.", failures);
  Expect(std::is_class_v<LegacyLazySqliteReportQueryService>,
         "Legacy LazySqliteReportQueryService header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyDailyReportService>,
         "Legacy DailyReportService header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyMonthlyReportService>,
         "Legacy MonthlyReportService header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyWeeklyReportService>,
         "Legacy WeeklyReportService header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyYearlyReportService>,
         "Legacy YearlyReportService header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalReportService>,
         "Canonical ReportService header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalLazySqliteReportQueryService>,
         "Canonical LazySqliteReportQueryService header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalDailyReportService>,
         "Canonical DailyReportService header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalMonthlyReportService>,
         "Canonical MonthlyReportService header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalWeeklyReportService>,
         "Canonical WeeklyReportService header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalYearlyReportService>,
         "Canonical YearlyReportService header contract should be visible.",
         failures);

  const auto legacy_run_period_query = &LegacyReportService::RunPeriodQuery;
  const auto canonical_run_period_query =
      &CanonicalReportService::RunPeriodQuery;
  const auto legacy_run_export_all_period =
      &LegacyReportService::RunExportAllPeriodReportsQuery;
  const auto canonical_run_export_all_period =
      &CanonicalReportService::RunExportAllPeriodReportsQuery;
  const auto legacy_generate_all_daily =
      &LegacyDailyReportService::GenerateAllReports;
  const auto canonical_generate_all_monthly =
      &CanonicalMonthlyReportService::GenerateReports;
  const auto legacy_generate_all_weekly =
      &LegacyWeeklyReportService::GenerateReports;
  const auto canonical_generate_all_yearly =
      &CanonicalYearlyReportService::GenerateReports;
  (void)legacy_run_period_query;
  (void)canonical_run_period_query;
  (void)legacy_run_export_all_period;
  (void)canonical_run_export_all_period;
  (void)legacy_generate_all_daily;
  (void)canonical_generate_all_monthly;
  (void)legacy_generate_all_weekly;
  (void)canonical_generate_all_yearly;

  std::error_code cleanup_error;
  const std::filesystem::path query_root =
      std::filesystem::path("temp") / "phase17_reports_querying_headers";
  const std::filesystem::path db_path = query_root / "reports.sqlite";
  std::filesystem::remove_all(query_root, cleanup_error);
  std::filesystem::create_directories(query_root, cleanup_error);

  try {
    infrastructure::persistence::importer::sqlite::Connection connection(
        db_path.string());
    ReportCatalog legacy_catalog = BuildMinimalReportCatalog();
    ReportCatalog canonical_catalog = BuildMinimalReportCatalog();

    LegacyReportService legacy_service(
        connection.GetDb(), legacy_catalog,
        std::make_shared<CompatPlatformClock>());
    CanonicalReportService canonical_service(
        connection.GetDb(), canonical_catalog,
        std::make_shared<CompatPlatformClock>());
    Expect(legacy_service
               .RunExportAllPeriodReportsQuery({}, ReportFormat::kMarkdown)
               .empty(),
           "Legacy ReportService should keep empty period export behavior.",
           failures);
    Expect(canonical_service
               .RunExportAllPeriodReportsQuery({}, ReportFormat::kMarkdown)
               .empty(),
           "Canonical ReportService should keep empty period export behavior.",
           failures);

    auto legacy_catalog_ptr =
        std::make_shared<ReportCatalog>(BuildMinimalReportCatalog());
    auto canonical_catalog_ptr =
        std::make_shared<ReportCatalog>(BuildMinimalReportCatalog());
    LegacyLazySqliteReportQueryService legacy_lazy_service(
        db_path, legacy_catalog_ptr, std::make_shared<CompatPlatformClock>());
    CanonicalLazySqliteReportQueryService canonical_lazy_service(
        db_path, canonical_catalog_ptr, std::make_shared<CompatPlatformClock>());
    Expect(legacy_lazy_service
               .RunExportAllPeriodReportsQuery({}, ReportFormat::kMarkdown)
               .empty(),
           "Legacy LazySqliteReportQueryService should keep empty export behavior.",
           failures);
    Expect(canonical_lazy_service
               .RunExportAllPeriodReportsQuery({}, ReportFormat::kMarkdown)
               .empty(),
           "Canonical LazySqliteReportQueryService should keep empty export behavior.",
           failures);
  } catch (...) {
    Expect(false,
           "Reports querying legacy/canonical adapters should construct and query successfully.",
           failures);
  }

  std::filesystem::remove_all(query_root, cleanup_error);
}

auto TestLegacyReportsDataQueryingHeaders(int& failures) -> void {
  using CanonicalLazySqliteReportDataQueryService =
      tracer::core::infrastructure::reports::LazySqliteReportDataQueryService;
  using CanonicalSqliteReportDataQueryService =
      tracer::core::infrastructure::reports::SqliteReportDataQueryService;
  using LegacyLazySqliteReportDataQueryService =
      infrastructure::reports::LazySqliteReportDataQueryService;
  using LegacySqliteReportDataQueryService =
      infrastructure::reports::SqliteReportDataQueryService;

  Expect(std::is_class_v<LegacySqliteReportDataQueryService>,
         "Legacy SqliteReportDataQueryService header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyLazySqliteReportDataQueryService>,
         "Legacy LazySqliteReportDataQueryService header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalSqliteReportDataQueryService>,
         "Canonical SqliteReportDataQueryService header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalLazySqliteReportDataQueryService>,
         "Canonical LazySqliteReportDataQueryService header contract should be visible.",
         failures);

  const auto legacy_query_all_daily =
      &LegacySqliteReportDataQueryService::QueryAllDaily;
  const auto canonical_query_all_daily =
      &CanonicalSqliteReportDataQueryService::QueryAllDaily;
  const auto legacy_query_period_batch =
      &LegacySqliteReportDataQueryService::QueryPeriodBatch;
  const auto canonical_query_all_yearly =
      &CanonicalLazySqliteReportDataQueryService::QueryAllYearly;
  (void)legacy_query_all_daily;
  (void)canonical_query_all_daily;
  (void)legacy_query_period_batch;
  (void)canonical_query_all_yearly;

  std::error_code cleanup_error;
  const std::filesystem::path query_root =
      std::filesystem::path("temp") / "phase18_reports_data_querying_headers";
  const std::filesystem::path db_path = query_root / "reports.sqlite";
  std::filesystem::remove_all(query_root, cleanup_error);
  std::filesystem::create_directories(query_root, cleanup_error);

  try {
    infrastructure::persistence::importer::sqlite::Connection connection(
        db_path.string());
    LegacySqliteReportDataQueryService legacy_service(
        connection.GetDb(), std::make_shared<CompatPlatformClock>());
    CanonicalSqliteReportDataQueryService canonical_service(
        connection.GetDb(), std::make_shared<CompatPlatformClock>());
    Expect(legacy_service.QueryAllDaily().empty(),
           "Legacy SqliteReportDataQueryService should keep empty daily batch behavior.",
           failures);
    Expect(canonical_service.QueryAllMonthly().empty(),
           "Canonical SqliteReportDataQueryService should keep empty monthly batch behavior.",
           failures);
    Expect(canonical_service.QueryPeriodBatch({}).empty(),
           "Canonical SqliteReportDataQueryService should keep empty period batch behavior.",
           failures);

    LegacyLazySqliteReportDataQueryService legacy_lazy_service(
        db_path, std::make_shared<CompatPlatformClock>());
    CanonicalLazySqliteReportDataQueryService canonical_lazy_service(
        db_path, std::make_shared<CompatPlatformClock>());
    Expect(legacy_lazy_service.QueryAllWeekly().empty(),
           "Legacy LazySqliteReportDataQueryService should keep empty weekly batch behavior.",
           failures);
    Expect(canonical_lazy_service.QueryAllYearly().empty(),
           "Canonical LazySqliteReportDataQueryService should keep empty yearly batch behavior.",
           failures);
  } catch (...) {
    Expect(false,
           "Reports data-querying legacy/canonical adapters should construct and query successfully.",
           failures);
  }

  std::filesystem::remove_all(query_root, cleanup_error);
}

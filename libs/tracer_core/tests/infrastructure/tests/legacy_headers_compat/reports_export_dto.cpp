#include "infrastructure/tests/legacy_headers_compat/support.hpp"

namespace {

auto BuildMinimalReportCatalog() -> ReportCatalog {
  ReportCatalog catalog;
  catalog.loaded_reports.markdown.day.labels.date_label = "Date";
  return catalog;
}

}  // namespace

auto TestLegacyReportsExportHeaders(int& failures) -> void {
  using CanonicalExporter = tracer::core::infrastructure::reports::Exporter;
  using CanonicalReportFileManager =
      tracer::core::infrastructure::reports::ReportFileManager;
  using CanonicalReportFormatDetails =
      tracer::core::infrastructure::reports::ReportFormatDetails;
  using LegacyExporter = ::Exporter;
  using LegacyReportFileManager = ::ReportFileManager;

  Expect(std::is_class_v<LegacyExporter>,
         "Legacy Exporter header path should remain visible.", failures);
  Expect(std::is_class_v<LegacyReportFileManager>,
         "Legacy ReportFileManager header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalExporter>,
         "Canonical Exporter header contract should be visible.", failures);
  Expect(std::is_class_v<CanonicalReportFileManager>,
         "Canonical ReportFileManager header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalReportFormatDetails>,
         "Canonical ReportFormatDetails header contract should be visible.",
         failures);

  const auto legacy_get_report_format_details =
      &ExportUtils::GetReportFormatDetails;
  const auto canonical_get_report_format_details =
      &tracer::core::infrastructure::reports::GetReportFormatDetails;
  const auto legacy_execute_export_task = &ExportUtils::ExecuteExportTask;
  const auto canonical_execute_export_task =
      &tracer::core::infrastructure::reports::ExecuteExportTask;
  (void)legacy_get_report_format_details;
  (void)canonical_get_report_format_details;
  (void)legacy_execute_export_task;
  (void)canonical_execute_export_task;

  const auto markdown_details =
      ExportUtils::GetReportFormatDetails(ReportFormat::kMarkdown);
  Expect(markdown_details.has_value() &&
             markdown_details->dir_name == "markdown" &&
             markdown_details->extension == ".md",
         "Legacy ExportUtils::GetReportFormatDetails should remain visible.",
         failures);

  std::error_code cleanup_error;
  const std::filesystem::path legacy_root =
      std::filesystem::path("temp") / "phase16_legacy_reports_export";
  const std::filesystem::path canonical_root =
      std::filesystem::path("temp") / "phase16_canonical_reports_export";
  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);

  LegacyReportFileManager legacy_file_manager(legacy_root);
  CanonicalReportFileManager canonical_file_manager(canonical_root);
  Expect(legacy_file_manager
                 .GetSingleDayReportPath("20260203", ReportFormat::kMarkdown)
                 .filename() == "2026-02-03.md",
         "Legacy ReportFileManager should normalize compact day labels.",
         failures);
  Expect(canonical_file_manager
                 .GetSingleMonthReportPath("202602", ReportFormat::kLaTeX)
                 .extension() == ".tex",
         "Canonical ReportFileManager should resolve report extensions.",
         failures);

  CanonicalExporter canonical_exporter(canonical_root);
  LegacyExporter legacy_exporter(legacy_root);
  legacy_exporter.ExportSingleDayReport(
      {.id = "2026-02-03", .kContent = "legacy export\n"},
      ReportFormat::kMarkdown);
  canonical_exporter.ExportSinglePeriodReport(
      7, "canonical export\n", ReportFormat::kMarkdown);

  Expect(std::filesystem::exists(
             legacy_root / "markdown" / "day" / "2026" / "02" /
             "2026-02-03.md"),
         "Legacy Exporter should write day exports to the expected path.",
         failures);
  Expect(std::filesystem::exists(
             canonical_root / "markdown" / "recent" / "last_7_days_report.md"),
         "Canonical Exporter should write period exports to the expected path.",
         failures);

  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);
}

auto TestLegacyReportsDtoHeaders(int& failures) -> void {
  using CanonicalReportDtoExportWriter =
      tracer::core::infrastructure::reports::ReportDtoExportWriter;
  using CanonicalReportDtoFormatter =
      tracer::core::infrastructure::reports::ReportDtoFormatter;
  using CanonicalExporter = tracer::core::infrastructure::reports::Exporter;
  using CanonicalReportFileManager =
      tracer::core::infrastructure::reports::ReportFileManager;
  using LegacyReportDtoExportWriter =
      infrastructure::reports::ReportDtoExportWriter;
  using LegacyReportDtoFormatter = infrastructure::reports::ReportDtoFormatter;
  using LegacyExporter = ::Exporter;
  using LegacyReportFileManager = ::ReportFileManager;

  Expect(std::is_class_v<LegacyReportDtoFormatter>,
         "Legacy ReportDtoFormatter header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyReportDtoExportWriter>,
         "Legacy ReportDtoExportWriter header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalReportDtoFormatter>,
         "Canonical ReportDtoFormatter header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalReportDtoExportWriter>,
         "Canonical ReportDtoExportWriter header contract should be visible.",
         failures);

  const auto legacy_format_daily = &LegacyReportDtoFormatter::FormatDaily;
  const auto canonical_format_daily = &CanonicalReportDtoFormatter::FormatDaily;
  const auto legacy_export_single_day =
      &LegacyReportDtoExportWriter::ExportSingleDay;
  const auto canonical_export_all_yearly =
      &CanonicalReportDtoExportWriter::ExportAllYearly;
  (void)legacy_format_daily;
  (void)canonical_format_daily;
  (void)legacy_export_single_day;
  (void)canonical_export_all_yearly;

  ReportCatalog legacy_catalog = BuildMinimalReportCatalog();
  ReportCatalog canonical_catalog = BuildMinimalReportCatalog();
  LegacyReportDtoFormatter legacy_formatter(legacy_catalog);
  CanonicalReportDtoFormatter canonical_formatter(canonical_catalog);
  (void)legacy_formatter;
  (void)canonical_formatter;

  std::error_code cleanup_error;
  const std::filesystem::path legacy_root =
      std::filesystem::path("temp") / "phase16_legacy_reports_dto";
  const std::filesystem::path canonical_root =
      std::filesystem::path("temp") / "phase16_canonical_reports_dto";
  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);

  LegacyReportFileManager legacy_file_manager(legacy_root);
  CanonicalReportFileManager canonical_file_manager(canonical_root);
  auto legacy_exporter = std::make_shared<LegacyExporter>(legacy_root);
  auto canonical_exporter = std::make_shared<CanonicalExporter>(canonical_root);
  auto legacy_formatter_ptr =
      std::make_shared<LegacyReportDtoFormatter>(legacy_catalog);
  auto canonical_formatter_ptr =
      std::make_shared<CanonicalReportDtoFormatter>(canonical_catalog);

  LegacyReportDtoExportWriter legacy_writer(legacy_formatter_ptr,
                                            legacy_exporter);
  CanonicalReportDtoExportWriter canonical_writer(canonical_formatter_ptr,
                                                  canonical_exporter);

  DailyReportData daily_report;
  daily_report.date = "2026-02-03";
  daily_report.total_duration = 60;
  legacy_writer.ExportSingleDay("2026-02-03", daily_report,
                                ReportFormat::kMarkdown);
  Expect(std::filesystem::exists(legacy_file_manager.GetSingleDayReportPath(
             "2026-02-03", ReportFormat::kMarkdown)),
         "Legacy ReportDtoExportWriter should keep export wiring visible.",
         failures);

  YearlyReportData yearly_report;
  yearly_report.range_label = "2026";
  yearly_report.total_duration = 60;
  canonical_writer.ExportAllYearly({{"2026", yearly_report}},
                                   ReportFormat::kMarkdown);
  Expect(std::filesystem::exists(canonical_file_manager.GetSingleYearReportPath(
             "2026", ReportFormat::kMarkdown)),
         "Canonical ReportDtoExportWriter should keep bulk export wiring visible.",
         failures);

  std::filesystem::remove_all(legacy_root, cleanup_error);
  std::filesystem::remove_all(canonical_root, cleanup_error);
}

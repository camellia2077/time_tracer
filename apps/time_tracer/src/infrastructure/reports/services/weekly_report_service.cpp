// infrastructure/reports/services/weekly_report_service.cpp
#include "infrastructure/reports/services/weekly_report_service.hpp"

#include <stdexcept>

#include "infrastructure/reports/data/queriers/weekly/weekly_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"
#include "shared/utils/period_utils.hpp"

WeeklyReportService::WeeklyReportService(sqlite3* database_connection,
                                         const ReportCatalog& report_catalog)
    : db_(database_connection), report_catalog_(report_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto WeeklyReportService::GenerateReports(ReportFormat format)
    -> FormattedWeeklyReports {
  FormattedWeeklyReports reports;

  auto& name_cache = reports::services::EnsureProjectNameCache(db_);

  BatchWeekDataFetcher fetcher(db_);
  auto all_weeks_data = fetcher.FetchAllData();

  auto formatter = GenericFormatterFactory<WeeklyReportData>::Create(
      format, report_catalog_);

  reports::services::FormatReportMap(
      all_weeks_data, formatter, name_cache,
      [&](const std::string& week_label,
          const std::string& formatted_report) -> void {
        IsoWeek parsed{};
        if (ParseIsoWeek(week_label, parsed)) {
          reports[parsed.year][parsed.week] = formatted_report;
        }
      });

  return reports;
}

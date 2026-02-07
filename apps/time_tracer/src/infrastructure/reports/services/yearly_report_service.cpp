// infrastructure/reports/services/yearly_report_service.cpp
#include "infrastructure/reports/services/yearly_report_service.hpp"

#include <stdexcept>

#include "infrastructure/reports/data/queriers/yearly/batch_year_data_fetcher.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"
#include "infrastructure/reports/shared/utils/format/year_utils.hpp"

YearlyReportService::YearlyReportService(sqlite3* sqlite_db,
                                         const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto YearlyReportService::GenerateReports(ReportFormat format)
    -> FormattedYearlyReports {
  FormattedYearlyReports reports;

  auto& name_cache = reports::services::EnsureProjectNameCache(db_);

  BatchYearDataFetcher fetcher(db_);
  auto all_years_data = fetcher.FetchAllData();

  auto formatter =
      GenericFormatterFactory<YearlyReportData>::Create(format, app_config_);

  reports::services::FormatReportMap(
      all_years_data, formatter, name_cache,
      [&](const std::string& year_label,
          const std::string& formatted_report) -> void {
        int gregorian_year = 0;
        if (ParseGregorianYear(year_label, gregorian_year)) {
          reports[gregorian_year] = formatted_report;
        }
      });

  return reports;
}

#include "infrastructure/sqlite_fwd.hpp"

#include <stdexcept>
#include <string>

#include "infrastructure/reports/services/daily_report_service.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/data/queriers/daily/daily_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.reports.models.query_data_structs;
import tracer.core.domain.reports.types.report_types;

namespace tracer::core::infrastructure::reports::services {

DailyReportService::DailyReportService(sqlite3* sqlite_db,
                                       const ReportCatalog& report_catalog)
    : db_(sqlite_db), report_catalog_(report_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto DailyReportService::GenerateAllReports(ReportFormat format)
    -> FormattedGroupedReports {
  FormattedGroupedReports grouped_reports;

  ProjectNameCache name_cache =
      ::reports::services::CreateProjectNameCache(db_);

  BatchDayDataFetcher fetcher(db_, name_cache);
  BatchDataResult batch_data = fetcher.FetchAllData();

  auto formatter =
      GenericFormatterFactory<DailyReportData>::Create(format, report_catalog_);

  for (const auto& [date, year, month] : batch_data.date_order) {
    DailyReportData& data = batch_data.data_map[date];

    if (data.total_duration > 0) {
      ::reports::services::EnsureProjectTree(data, name_cache);

      std::string formatted_report = formatter->FormatReport(data);
      grouped_reports[year][month].push_back({date, formatted_report});
    }
  }

  return grouped_reports;
}

}  // namespace tracer::core::infrastructure::reports::services

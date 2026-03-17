#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <stdexcept>
#include <string>

#include "infrastructure/reports/services/weekly_report_service.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/data/queriers/weekly/weekly_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.reports.models.query_data_structs;
import tracer.core.domain.reports.types.report_types;
import tracer.core.shared.period_utils;

namespace modperiod = tracer::core::shared::modperiod;
namespace tracer::core::infrastructure::reports::services {

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

  ProjectNameCache name_cache =
      ::reports::services::CreateProjectNameCache(db_);

  BatchWeekDataFetcher fetcher(db_);
  auto all_weeks_data = fetcher.FetchAllData();

  auto formatter = GenericFormatterFactory<WeeklyReportData>::Create(
      format, report_catalog_);

  ::reports::services::FormatReportMap(
      all_weeks_data, formatter, name_cache,
      [&](const std::string& week_label,
          const std::string& formatted_report) -> void {
        modperiod::IsoWeek parsed{};
        if (modperiod::ParseIsoWeek(week_label, parsed)) {
          reports[parsed.year][parsed.week] = formatted_report;
        }
      });

  return reports;
}

}  // namespace tracer::core::infrastructure::reports::services

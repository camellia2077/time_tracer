#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <stdexcept>
#include <string>
#include <utility>

#include "infrastructure/reports/services/monthly_report_service.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/data/queriers/monthly/monthly_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.reports.models.query_data_structs;
import tracer.core.domain.reports.types.report_types;

namespace {
constexpr size_t kYearMonthStrMinLen = 7;
constexpr size_t kYearMonthYearLen = 4;
constexpr size_t kYearMonthDashPos = 4;
constexpr size_t kYearMonthMonthPos = 5;
constexpr size_t kYearMonthMonthLen = 2;
}  // namespace

static auto ParseYearMonth(const std::string& year_month_str)
    -> std::pair<int, int> {
  if (year_month_str.size() >= kYearMonthStrMinLen &&
      year_month_str[kYearMonthDashPos] == '-') {
    try {
      int year = std::stoi(year_month_str.substr(0, kYearMonthYearLen));
      int month = std::stoi(
          year_month_str.substr(kYearMonthMonthPos, kYearMonthMonthLen));
      return {year, month};
    } catch (...) {
      return {0, 0};
    }
  }
  return {0, 0};
}

namespace tracer::core::infrastructure::reports::services {

MonthlyReportService::MonthlyReportService(sqlite3* database_connection,
                                           const ReportCatalog& report_catalog)
    : db_(database_connection), report_catalog_(report_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto MonthlyReportService::GenerateReports(ReportFormat format)
    -> FormattedMonthlyReports {
  FormattedMonthlyReports reports;

  ProjectNameCache name_cache =
      ::reports::services::CreateProjectNameCache(db_);

  BatchMonthDataFetcher fetcher(db_);
  auto all_months_data = fetcher.FetchAllData();

  auto formatter = GenericFormatterFactory<MonthlyReportData>::Create(
      format, report_catalog_);

  ::reports::services::FormatReportMap(
      all_months_data, formatter, name_cache,
      [&](const std::string& year_month_str,
          const std::string& formatted_report) -> void {
        auto [year, month] = ParseYearMonth(year_month_str);
        if (year > 0 && month > 0) {
          reports[year][month] = formatted_report;
        }
      });

  return reports;
}

}  // namespace tracer::core::infrastructure::reports::services

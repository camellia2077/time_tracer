// reports/services/yearly_report_service.cpp
#include "yearly_report_service.hpp"

#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/queriers/yearly/batch_year_data_fetcher.hpp"
#include "reports/data/utils/project_tree_builder.hpp"
#include "reports/shared/factories/generic_formatter_factory.hpp"
#include "reports/shared/utils/format/year_utils.hpp"

YearlyReportService::YearlyReportService(sqlite3* sqlite_db,
                                         const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto YearlyReportService::generate_reports(ReportFormat format)
    -> FormattedYearlyReports {
  FormattedYearlyReports reports;

  auto& name_cache = ProjectNameCache::instance();
  name_cache.ensure_loaded(db_);

  BatchYearDataFetcher fetcher(db_);
  auto all_years_data = fetcher.fetch_all_data();

  auto formatter =
      GenericFormatterFactory<YearlyReportData>::create(format, app_config_);

  for (auto& [year_label, data] : all_years_data) {
    if (data.total_duration > 0) {
      build_project_tree_from_ids(data.project_tree, data.project_stats,
                                  name_cache);

      std::string formatted_report = formatter->format_report(data);

      int gregorian_year = 0;
      if (parse_gregorian_year(year_label, gregorian_year)) {
        reports[gregorian_year] = formatted_report;
      }
    }
  }

  return reports;
}

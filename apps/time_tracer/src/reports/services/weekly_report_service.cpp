// reports/services/weekly_report_service.cpp
#include "weekly_report_service.hpp"

#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/queriers/weekly/batch_week_data_fetcher.hpp"
#include "reports/data/utils/project_tree_builder.hpp"
#include "reports/shared/factories/generic_formatter_factory.hpp"
#include "reports/shared/utils/format/iso_week_utils.hpp"

WeeklyReportService::WeeklyReportService(sqlite3* sqlite_db,
                                         const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto WeeklyReportService::generate_reports(ReportFormat format)
    -> FormattedWeeklyReports {
  FormattedWeeklyReports reports;

  auto& name_cache = ProjectNameCache::instance();
  name_cache.ensure_loaded(db_);

  BatchWeekDataFetcher fetcher(db_);
  auto all_weeks_data = fetcher.fetch_all_data();

  auto formatter =
      GenericFormatterFactory<WeeklyReportData>::create(format, app_config_);

  for (auto& [week_label, data] : all_weeks_data) {
    if (data.total_duration > 0) {
      build_project_tree_from_ids(data.project_tree, data.project_stats,
                                  name_cache);

      std::string formatted_report = formatter->format_report(data);

      IsoWeek parsed{};
      if (parse_iso_week(week_label, parsed)) {
        reports[parsed.year][parsed.week] = formatted_report;
      }
    }
  }

  return reports;
}

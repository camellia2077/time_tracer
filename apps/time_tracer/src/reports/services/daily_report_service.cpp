// reports/services/daily_report_service.cpp
#include "reports/services/daily_report_service.hpp"
// [修改] 指向新的 data 模块路径
#include <stdexcept>

#include "reports/data/queriers/daily/batch_day_data_fetcher.hpp"
#include "reports/services/batch_export_helpers.hpp"
#include "reports/shared/factories/generic_formatter_factory.hpp"

DailyReportService::DailyReportService(sqlite3* sqlite_db,
                                       const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto DailyReportService::generate_all_reports(ReportFormat format)
    -> FormattedGroupedReports {
  FormattedGroupedReports grouped_reports;

  // [新增] 准备项目名称缓存
  auto& name_cache = reports::services::EnsureProjectNameCache(db_);

  // 1. 委托 Fetcher 获取所有准备好的数据
  // [注意] BatchDayDataFetcher 的构造函数现在需要 IProjectInfoProvider
  BatchDayDataFetcher fetcher(db_, name_cache);
  BatchDataResult batch_data = fetcher.FetchAllData();

  // 2. 创建格式化器
  auto formatter =
      GenericFormatterFactory<DailyReportData>::Create(format, app_config_);

  // 3. 遍历并格式化
  for (const auto& [date, year, month] : batch_data.date_order) {
    DailyReportData& data = batch_data.data_map[date];

    if (data.total_duration > 0) {
      // [修改] 传入 name_cache 替代 db_
      reports::services::EnsureProjectTree(data, name_cache);

      // 格式化
      std::string formatted_report = formatter->FormatReport(data);
      grouped_reports[year][month].emplace_back(date, formatted_report);
    }
  }

  return grouped_reports;
}

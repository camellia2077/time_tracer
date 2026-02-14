// infrastructure/reports/services/daily_report_service.cpp
#include "infrastructure/reports/services/daily_report_service.hpp"
// [修改] 指向新的 data 模块路径
#include <stdexcept>

#include "infrastructure/reports/data/queriers/daily/daily_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

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

  // [新增] 准备项目名称缓存
  auto& name_cache = reports::services::EnsureProjectNameCache(db_);

  // 1. 委托 Fetcher 获取所有准备好的数据
  // [注意] BatchDayDataFetcher 的构造函数现在需要 IProjectInfoProvider
  BatchDayDataFetcher fetcher(db_, name_cache);
  BatchDataResult batch_data = fetcher.FetchAllData();

  // 2. 创建格式化器
  auto formatter =
      GenericFormatterFactory<DailyReportData>::Create(format, report_catalog_);

  // 3. 遍历并格式化
  for (const auto& [date, year, month] : batch_data.date_order) {
    DailyReportData& data = batch_data.data_map[date];

    if (data.total_duration > 0) {
      // [修改] 传入 name_cache 替代 db_
      reports::services::EnsureProjectTree(data, name_cache);

      // 格式化
      std::string formatted_report = formatter->FormatReport(data);
      grouped_reports[year][month].push_back({date, formatted_report});
    }
  }

  return grouped_reports;
}

// reports/services/monthly_report_service.cpp
#include "monthly_report_service.hpp"
// [修改] 指向新的 data 模块路径
#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/queriers/monthly/batch_month_data_fetcher.hpp"
#include "reports/data/utils/project_tree_builder.hpp"
#include "reports/shared/factories/generic_formatter_factory.hpp"

namespace {
constexpr size_t kYearMonthStrMinLen = 7;
constexpr size_t kYearMonthYearLen = 4;
constexpr size_t kYearMonthDashPos = 4;
constexpr size_t kYearMonthMonthPos = 5;
constexpr size_t kYearMonthMonthLen = 2;
}  // namespace

// 辅助函数保持不变
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

MonthlyReportService::MonthlyReportService(sqlite3* sqlite_db,
                                           const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto MonthlyReportService::generate_reports(ReportFormat format)
    -> FormattedMonthlyReports {
  FormattedMonthlyReports reports;

  // [新增] 准备项目名称缓存
  auto& name_cache = ProjectNameCache::instance();
  name_cache.ensure_loaded(db_);

  // 1. 委托 Fetcher 获取所有数据
  BatchMonthDataFetcher fetcher(db_);
  auto all_months_data = fetcher.fetch_all_data();

  // 2. 创建格式化器
  auto formatter =
      GenericFormatterFactory<MonthlyReportData>::create(format, app_config_);

  // 3. 遍历并格式化
  for (auto& [year_month_str, data] : all_months_data) {
    if (data.total_duration > 0) {
      // 3.1 构建项目树
      // [修改] 传入 name_cache 替代 db_
      build_project_tree_from_ids(data.project_tree, data.project_stats,
                                  name_cache);

      // 3.2 格式化
      std::string formatted_report = formatter->format_report(data);

      // 3.3 整理到返回结构中
      auto [year, month] = ParseYearMonth(year_month_str);
      if (year > 0 && month > 0) {
        reports[year][month] = formatted_report;
      }
    }
  }

  return reports;
}
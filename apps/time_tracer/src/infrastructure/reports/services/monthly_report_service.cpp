// infrastructure/reports/services/monthly_report_service.cpp
#include "infrastructure/reports/services/monthly_report_service.hpp"
// [修改] 指向新的 data 模块路径
#include <stdexcept>

#include "infrastructure/reports/data/queriers/monthly/monthly_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

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

  // [新增] 准备项目名称缓存
  auto& name_cache = reports::services::EnsureProjectNameCache(db_);

  // 1. 委托 Fetcher 获取所有数据
  BatchMonthDataFetcher fetcher(db_);
  auto all_months_data = fetcher.FetchAllData();

  // 2. 创建格式化器
  auto formatter = GenericFormatterFactory<MonthlyReportData>::Create(
      format, report_catalog_);

  // 3. 遍历并格式化
  reports::services::FormatReportMap(
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

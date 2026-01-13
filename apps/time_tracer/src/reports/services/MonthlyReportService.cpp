// reports/services/MonthlyReportService.cpp
#include "MonthlyReportService.hpp"
#include "reports/monthly/queriers/BatchMonthDataFetcher.hpp" // [新增]
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/shared/utils/tree/ProjectTreeBuilder.hpp"
#include <stdexcept>

// 辅助函数保持不变
static std::pair<int, int> parse_year_month(const std::string& ym) {
    if (ym.size() >= 7 && ym[4] == '-') {
        try {
            int y = std::stoi(ym.substr(0, 4));
            int m = std::stoi(ym.substr(5, 2));
            return {y, m};
        } catch (...) {
            return {0, 0};
        }
    }
    return {0, 0};
}

MonthlyReportService::MonthlyReportService(sqlite3* db, const AppConfig& config) 
    : db_(db), app_config_(config) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedMonthlyReports MonthlyReportService::generate_reports(ReportFormat format) {
    FormattedMonthlyReports reports;

    // 1. 委托 Fetcher 获取所有数据 (核心解耦)
    //    这里不再包含 SQL 代码
    BatchMonthDataFetcher fetcher(db_);
    auto all_months_data = fetcher.fetch_all_data();

    // 2. 创建格式化器
    auto formatter = GenericFormatterFactory<MonthlyReportData>::create(format, app_config_);

    // 3. 遍历并格式化
    for (auto& [ym, data] : all_months_data) {
        if (data.total_duration > 0) {
            // 3.1 构建项目树 (纯内存操作，依赖缓存)
            // 注意: data.project_stats 已经在 Fetcher 里填充好了
            build_project_tree_from_ids(data.project_tree, data.project_stats, db_);
            
            // 3.2 格式化
            std::string formatted_report = formatter->format_report(data);

            // 3.3 整理到返回结构中
            auto [year, month] = parse_year_month(ym);
            if (year > 0 && month > 0) {
                reports[year][month] = formatted_report;
            }
        }
    }

    return reports;
}
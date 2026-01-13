// reports/services/PeriodReportService.cpp
#include "PeriodReportService.hpp"
#include "reports/period/queriers/BatchPeriodDataFetcher.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/shared/utils/tree/ProjectTreeBuilder.hpp"
#include <stdexcept>
#include <memory>

PeriodReportService::PeriodReportService(sqlite3* db, const AppConfig& config) 
    : db_(db), app_config_(config) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports PeriodReportService::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;

    // 1. 批量获取数据 (核心优化)
    BatchPeriodDataFetcher fetcher(db_);
    std::map<int, PeriodReportData> all_data = fetcher.fetch_all_data(days_list);

    // 2. 创建格式化器
    auto formatter = GenericFormatterFactory<PeriodReportData>::create(format, app_config_);

    // 3. 遍历并格式化
    for (auto& [days, data] : all_data) {
        if (days > 0 && data.total_duration > 0) {
            // 3.1 构建项目树 (利用缓存 + 内存构建)
            // data.project_stats 已经在 Fetcher 中聚合完毕
            build_project_tree_from_ids(data.project_tree, data.project_stats, db_);

            // 3.2 格式化
            std::string formatted_report = formatter->format_report(data);
            reports[days] = formatted_report;
        }
    }

    return reports;
}
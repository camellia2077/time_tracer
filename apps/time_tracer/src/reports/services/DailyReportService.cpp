// reports/services/DailyReportService.cpp
#include "DailyReportService.hpp"
#include "reports/daily/queriers/BatchDayDataFetcher.hpp" // [新增]
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/shared/utils/tree/ProjectTreeBuilder.hpp"
#include <stdexcept>

DailyReportService::DailyReportService(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedGroupedReports DailyReportService::generate_all_reports(ReportFormat format) {
    FormattedGroupedReports grouped_reports;

    // 1. 委托 Fetcher 获取所有准备好的数据 (核心解耦)
    //    这里不再包含任何 SQL 代码
    BatchDayDataFetcher fetcher(db_);
    BatchDataResult batch_data = fetcher.fetch_all_data();

    // 2. 创建格式化器
    auto formatter = GenericFormatterFactory<DailyReportData>::create(format, app_config_);

    // 3. 遍历并格式化
    for (const auto& [date, year, month] : batch_data.date_order) {
        // 使用引用避免拷贝
        DailyReportData& data = batch_data.data_map[date];

        if (data.total_duration > 0) {
            // 构建项目树 (纯内存操作)
            // 注意: data.project_stats 已经在 Fetcher 里填充好了
            build_project_tree_from_ids(data.project_tree, data.project_stats, db_);
            
            // 格式化
            std::string formatted_report = formatter->format_report(data);
            grouped_reports[year][month].push_back({date, formatted_report});
        }
    }

    return grouped_reports;
}
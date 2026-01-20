// reports/services/period_report_service.cpp
#include "period_report_service.hpp"
// [修改] 指向新的 data 模块路径
#include "reports/data/queriers/period/batch_period_data_fetcher.hpp"
#include "reports/shared/factories/generic_formatter_factory.hpp"
#include "reports/data/utils/project_tree_builder.hpp"
#include "reports/data/cache/project_name_cache.hpp"
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

    // [新增] 准备项目名称缓存
    auto& name_cache = ProjectNameCache::instance();
    name_cache.ensure_loaded(db_);

    // 1. 批量获取数据
    BatchPeriodDataFetcher fetcher(db_);
    std::map<int, PeriodReportData> all_data = fetcher.fetch_all_data(days_list);

    // 2. 创建格式化器
    auto formatter = GenericFormatterFactory<PeriodReportData>::create(format, app_config_);

    // 3. 遍历并格式化
    for (auto& [days, data] : all_data) {
        if (days > 0 && data.total_duration > 0) {
            // 3.1 构建项目树
            // [修改] 传入 name_cache 替代 db_
            build_project_tree_from_ids(data.project_tree, data.project_stats, name_cache);

            // 3.2 格式化
            std::string formatted_report = formatter->format_report(data);
            reports[days] = formatted_report;
        }
    }

    return reports;
}
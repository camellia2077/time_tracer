// reports/services/daily_report_service.cpp
#include "daily_report_service.hpp"
#include "reports/daily/queriers/batch_day_data_fetcher.hpp"
#include "reports/shared/factories/generic_formatter_factory.hpp"
#include "reports/shared/utils/tree/project_tree_builder.hpp"
#include "reports/shared/cache/project_name_cache.hpp"
#include <stdexcept>

DailyReportService::DailyReportService(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedGroupedReports DailyReportService::generate_all_reports(ReportFormat format) {
    FormattedGroupedReports grouped_reports;

    // [新增] 准备项目名称缓存
    auto& name_cache = ProjectNameCache::instance();
    name_cache.ensure_loaded(db_);

    // 1. 委托 Fetcher 获取所有准备好的数据
    // [修改] 根据编译器报错，构造函数需要传入 provider
    BatchDayDataFetcher fetcher(db_, name_cache);
    BatchDataResult batch_data = fetcher.fetch_all_data();

    // 2. 创建格式化器
    auto formatter = GenericFormatterFactory<DailyReportData>::create(format, app_config_);

    // 3. 遍历并格式化
    for (const auto& [date, year, month] : batch_data.date_order) {
        DailyReportData& data = batch_data.data_map[date];

        if (data.total_duration > 0) {
            // [修改] 传入 name_cache 替代 db_
            build_project_tree_from_ids(data.project_tree, data.project_stats, name_cache);
            
            // 格式化
            std::string formatted_report = formatter->format_report(data);
            grouped_reports[year][month].push_back({date, formatted_report});
        }
    }

    return grouped_reports;
}
#include "AllPeriodReports.h"
#include "queries/period/PeriodReportFormatterFactory.h"
#include "queries/period/PeriodReportQuerier.h" // [修正] 添加缺失的头文件
#include <stdexcept>

AllPeriodReports::AllPeriodReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

// [修改] 更新函数签名以接收 ReportFormat 参数
FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;
    
    // [修改] 在循环外使用工厂创建一次格式化器实例
    auto formatter = PeriodReportFormatterFactory::create_formatter(format);

    for (int days : days_list) {
        if (days > 0) {
            // 步骤 1: 获取数据 (逻辑不变)
            PeriodReportQuerier querier(m_db, days);
            PeriodReportData report_data = querier.fetch_data();

            // 步骤 2: [修改] 使用在循环外创建的格式化器实例来格式化报告
            std::string formatted_report = formatter->format_report(report_data, m_db);

            // 步骤 3: 存储结果 (逻辑不变)
            reports[days] = formatted_report;
        }
    }

    return reports;
}
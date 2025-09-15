// queries/export/AllPeriodReports.cpp

#include "AllPeriodReports.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include <stdexcept>
#include <memory>

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp" // [新增] 引入周期报告Typst配置类

// [修改] 构造函数，接收数据库连接和周期报告Typst配置路径
AllPeriodReports::AllPeriodReports(sqlite3* db, const std::string& period_typ_config_path) 
    : m_db(db), m_period_typ_config_path(period_typ_config_path) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;
    
    std::unique_ptr<IReportFormatter<PeriodReportData>> formatter;
    if (format == ReportFormat::Typ) {
        // [修改] 如果格式为Typst，则使用配置路径来创建专门的格式化器
        auto config = std::make_shared<PeriodTypConfig>(m_period_typ_config_path);
        formatter = std::make_unique<PeriodTyp>(config);
    } else {
        // 对于其他格式，继续使用通用工厂
        formatter = ReportFmtFactory<PeriodReportData, PeriodMd, PeriodTex>::create_formatter(format);
    }

    for (int days : days_list) {
        if (days > 0) {
            PeriodQuerier querier(m_db, days);
            PeriodReportData report_data = querier.fetch_data();

            std::string formatted_report = formatter->format_report(report_data, m_db);
            reports[days] = formatted_report;
        }
    }

    return reports;
}
// reports/monthly/formatters/md/MonthMd.cpp
#include "MonthMd.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/MarkdownUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "reports/monthly/formatters/md/MonthMdConfig.hpp"      // [新增]
#include "reports/shared/data/MonthlyReportData.hpp"           // [新增]

// [新增] 自我注册逻辑
namespace {
    struct MonthMdRegister {
        MonthMdRegister() {
            GenericFormatterFactory<MonthlyReportData>::regist(ReportFormat::Markdown, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
                    auto md_config = std::make_shared<MonthMdConfig>(cfg.month_md_config_path);
                    return std::make_unique<MonthMd>(md_config);
                });
        }
    };
    const MonthMdRegister registrar;
}


MonthMd::MonthMd(std::shared_ptr<MonthMdConfig> config) : config_(config) {}

std::string MonthMd::format_report(const MonthlyReportData& data) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        ss << config_->get_invalid_format_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data);
    return ss.str();
}

void MonthMd::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << std::format("## {0} {1}-{2}\n\n", 
        config_->get_report_title(), 
        data.year_month.substr(0, 4), 
        data.year_month.substr(4, 2)
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

void MonthMd::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << MarkdownUtils::format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}
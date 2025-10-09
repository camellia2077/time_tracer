// reports/monthly/formatters/md/MonthMd.cpp
#include "MonthMd.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/MarkdownUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/monthly/formatters/md/MonthMdConfig.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include "common/AppConfig.hpp" // [新增] 为导出函数引入 AppConfig

// 移除了静态注册逻辑


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

// [新增] C-style functions to be exported from the DLL
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto md_config = std::make_shared<MonthMdConfig>(cfg.month_md_config_path);
        auto formatter = new MonthMd(md_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<MonthMd*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const MonthlyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<MonthMd*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
// reports/period/formatters/md/PeriodMd.cpp
#include "PeriodMd.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/MarkdownUtils.hpp"
#include "reports/period/formatters/md/PeriodMdConfig.hpp"
#include "reports/shared/data/PeriodReportData.hpp"
#include "common/AppConfig.hpp"

// [核心修改] 移除了静态自我注册逻辑

PeriodMd::PeriodMd(std::shared_ptr<PeriodMdConfig> config) : config_(config) {}

std::string PeriodMd::format_report(const PeriodReportData& data) const {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        ss << config_->get_invalid_days_message() << "\n";
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

void PeriodMd::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << std::format("## {0} {1} {2} ({3} {4} {5})\n\n",
        config_->get_report_title_prefix(),
        data.days_to_query,
        config_->get_report_title_days(),
        data.start_date,
        config_->get_report_title_date_separator(),
        data.end_date
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodMd::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    ss << MarkdownUtils::format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}

// [新增] C-style functions to be exported from the DLL
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto md_config = std::make_shared<PeriodMdConfig>(cfg.period_md_config_path);
        auto formatter = new PeriodMd(md_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<PeriodMd*>(handle);
        }
    }

    static std::string report_buffer;

    // 注意：这里的 format_report 需要一个新的函数指针类型来支持 PeriodReportData
    // 假设在 IReportFormatter.hpp 中已添加
    // typedef const char* (*FormatReportFunc_Period)(FormatterHandle, const PeriodReportData&);
    __declspec(dllexport) const char* format_report(FormatterHandle handle, const PeriodReportData& data) {
        if (handle) {
            auto* formatter = static_cast<PeriodMd*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
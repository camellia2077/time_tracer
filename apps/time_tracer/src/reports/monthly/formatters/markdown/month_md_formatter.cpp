// reports/monthly/formatters/markdown/month_md_formatter.cpp
#include "month_md_formatter.hpp"
#include <format>
#include "reports/shared/utils/format/time_format.hpp"
#include <toml++/toml.h>

MonthMdFormatter::MonthMdFormatter(std::shared_ptr<MonthMdConfig> config) 
    : BaseMdFormatter(config) {}

std::string MonthMdFormatter::validate_data(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message();
    }
    return "";
}

bool MonthMdFormatter::is_empty_data(const MonthlyReportData& data) const {
    return data.actual_days == 0;
}

int MonthMdFormatter::get_avg_days(const MonthlyReportData& data) const {
    return data.actual_days;
}

std::string MonthMdFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void MonthMdFormatter::format_header_content(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", 
        config_->get_report_title(), 
        data.year_month 
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_toml) {
        try {
            // [FIX] 使用 toml::parse
            auto config_tbl = toml::parse(config_toml);
            auto md_config = std::make_shared<MonthMdConfig>(config_tbl);
            auto formatter = new MonthMdFormatter(md_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<MonthMdFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const MonthlyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<MonthMdFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
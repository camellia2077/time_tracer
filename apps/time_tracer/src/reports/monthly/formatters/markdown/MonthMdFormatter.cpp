// reports/monthly/formatters/markdown/MonthMdFormatter.cpp
#include "MonthMdFormatter.hpp"
#include <format>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include <nlohmann/json.hpp>

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
    // [核心修改] 接收 config_json 字符串
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_json) {
        try {
            auto json_obj = nlohmann::json::parse(config_json);
            auto md_config = std::make_shared<MonthMdConfig>(json_obj);
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
// reports/monthly/formatters/latex/MonthTexFormatter.cpp
#include "MonthTexFormatter.hpp"
#include "MonthTexUtils.hpp"
#include <nlohmann/json.hpp>

MonthTexFormatter::MonthTexFormatter(std::shared_ptr<MonthTexConfig> config) 
    : BaseTexFormatter(config) {}

std::string MonthTexFormatter::validate_data(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
    }
    return "";
}

bool MonthTexFormatter::is_empty_data(const MonthlyReportData& data) const {
    return data.actual_days == 0;
}

int MonthTexFormatter::get_avg_days(const MonthlyReportData& data) const {
    return data.actual_days;
}

std::string MonthTexFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void MonthTexFormatter::format_header_content(std::stringstream& ss, const MonthlyReportData& data) const {
    MonthTexUtils::display_summary(ss, data, config_);
}

extern "C" {
    // [核心修改] 接收 config_json 字符串
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_json) {
        try {
            auto json_obj = nlohmann::json::parse(config_json);
            auto tex_config = std::make_shared<MonthTexConfig>(json_obj);
            auto formatter = new MonthTexFormatter(tex_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<MonthTexFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const MonthlyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<MonthTexFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
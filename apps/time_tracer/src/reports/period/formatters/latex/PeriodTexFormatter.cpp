// reports/period/formatters/latex/PeriodTexFormatter.cpp
#include "PeriodTexFormatter.hpp"
#include <format>
#include "reports/period/formatters/latex/PeriodTexUtils.hpp"
#include <toml++/toml.h> // [修改]

PeriodTexFormatter::PeriodTexFormatter(std::shared_ptr<PeriodTexConfig> config) 
    : BaseTexFormatter(config) {}

std::string PeriodTexFormatter::validate_data(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message();
    }
    return "";
}

bool PeriodTexFormatter::is_empty_data(const PeriodReportData& data) const {
    return data.actual_days == 0;
}

int PeriodTexFormatter::get_avg_days(const PeriodReportData& data) const {
    return data.actual_days;
}

std::string PeriodTexFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void PeriodTexFormatter::format_header_content(std::stringstream& ss, const PeriodReportData& data) const {
    PeriodTexUtils::display_summary(ss, data, config_);
}

extern "C" {
    // [核心修改] 使用 toml::parse
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_toml) {
        try {
            auto config_tbl = toml::parse(config_toml);
            auto tex_config = std::make_shared<PeriodTexConfig>(config_tbl);
            auto formatter = new PeriodTexFormatter(tex_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<PeriodTexFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const PeriodReportData& data) {
        if (handle) {
            auto* formatter = static_cast<PeriodTexFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
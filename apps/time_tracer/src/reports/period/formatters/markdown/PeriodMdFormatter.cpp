// reports/period/formatters/markdown/PeriodMdFormatter.cpp
#include "PeriodMdFormatter.hpp"
#include <format>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include <toml++/toml.h> // [修改]

PeriodMdFormatter::PeriodMdFormatter(std::shared_ptr<PeriodMdConfig> config) 
    : BaseMdFormatter(config) {}

std::string PeriodMdFormatter::validate_data(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message();
    }
    return "";
}

bool PeriodMdFormatter::is_empty_data(const PeriodReportData& data) const {
    return data.actual_days == 0;
}

int PeriodMdFormatter::get_avg_days(const PeriodReportData& data) const {
    return data.actual_days;
}

std::string PeriodMdFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void PeriodMdFormatter::format_header_content(std::stringstream& ss, const PeriodReportData& data) const {
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

extern "C" {
    // [核心修改] 使用 toml::parse
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_toml) {
        try {
            auto config_tbl = toml::parse(config_toml);
            auto md_config = std::make_shared<PeriodMdConfig>(config_tbl);
            auto formatter = new PeriodMdFormatter(md_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<PeriodMdFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const PeriodReportData& data) {
        if (handle) {
            auto* formatter = static_cast<PeriodMdFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
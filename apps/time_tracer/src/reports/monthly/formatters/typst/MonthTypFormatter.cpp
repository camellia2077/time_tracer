// reports/monthly/formatters/typst/MonthTypFormatter.cpp
#include "MonthTypFormatter.hpp"
#include <format>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "common/AppConfig.hpp"

MonthTypFormatter::MonthTypFormatter(std::shared_ptr<MonthTypConfig> config) 
    : BaseTypFormatter(config) {}

std::string MonthTypFormatter::validate_data(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message();
    }
    return "";
}

bool MonthTypFormatter::is_empty_data(const MonthlyReportData& data) const {
    return data.actual_days == 0;
}

int MonthTypFormatter::get_avg_days(const MonthlyReportData& data) const {
    return data.actual_days;
}

std::string MonthTypFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void MonthTypFormatter::format_page_setup(std::stringstream& ss) const {
    ss << std::format(R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
        config_->get_margin_top_cm(),
        config_->get_margin_bottom_cm(),
        config_->get_margin_left_cm(),
        config_->get_margin_right_cm()
    ) << "\n";
}

void MonthTypFormatter::format_header_content(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{}", size: {}pt)[= {} {}])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
        config_->get_report_title(),
        data.year_month 
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("+ *{}:* {}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto typ_config = std::make_shared<MonthTypConfig>(cfg.month_typ_config_path);
        auto formatter = new MonthTypFormatter(typ_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<MonthTypFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const MonthlyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<MonthTypFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
// reports/period/formatters/typst/PeriodTypFormatter.cpp
#include "PeriodTypFormatter.hpp"
#include <format>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include <toml++/toml.h> // [修改]

PeriodTypFormatter::PeriodTypFormatter(std::shared_ptr<PeriodTypConfig> config) 
    : BaseTypFormatter(config) {}

std::string PeriodTypFormatter::validate_data(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message();
    }
    return "";
}

bool PeriodTypFormatter::is_empty_data(const PeriodReportData& data) const {
    return data.actual_days == 0;
}

int PeriodTypFormatter::get_avg_days(const PeriodReportData& data) const {
    return data.actual_days;
}

std::string PeriodTypFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void PeriodTypFormatter::format_page_setup(std::stringstream& ss) const {
    ss << std::format(R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
        config_->get_margin_top_cm(),
        config_->get_margin_bottom_cm(),
        config_->get_margin_left_cm(),
        config_->get_margin_right_cm()
    ) << "\n";
}

void PeriodTypFormatter::format_header_content(std::stringstream& ss, const PeriodReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{}", size: {}pt)[= {} {} {} ({} {} {})])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
        config_->get_report_title_prefix(),
        data.days_to_query,
        config_->get_report_title_days(),
        data.start_date,
        config_->get_report_title_date_separator(),
        data.end_date
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("+ *{}:* {}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

extern "C" {
    // [核心修改] 使用 toml::parse
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_toml) {
        try {
            // [FIX] toml::parse 可能返回 parse_result，但在某些上下文中编译器可能将其视为 table
            // 这里我们使用 auto 并直接将其作为 table 传递，移除 ! 和 * 操作符以匹配编译器行为
            auto config_tbl = toml::parse(config_toml);
            
            // 假设 config_tbl 是一个 table 或可以隐式转换/行为像 table
            // 如果它是 parse_result，它包含 table。
            auto typ_config = std::make_shared<PeriodTypConfig>(config_tbl);
            auto formatter = new PeriodTypFormatter(typ_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<PeriodTypFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const PeriodReportData& data) {
        if (handle) {
            auto* formatter = static_cast<PeriodTypFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
// reports/monthly/formatters/typst/month_typ_formatter.cpp
#include "reports/monthly/formatters/typst/month_typ_formatter.hpp"
#include <format>
#include "reports/shared/utils/format/time_format.hpp"
#include <toml++/toml.h>
MonthTypFormatter::MonthTypFormatter(std::shared_ptr<MonthTypConfig> config) 
    : BaseTypFormatter(config) {}

auto MonthTypFormatter::ValidateData(const MonthlyReportData& data) const -> std::string {
    if (data.year_month == "INVALID") {
        return config_->GetInvalidFormatMessage();
    }
    return "";
}

auto MonthTypFormatter::IsEmptyData(const MonthlyReportData& data) const -> bool {
    return data.actual_days == 0;
}

auto MonthTypFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
    return data.actual_days;
}

auto MonthTypFormatter::GetNoRecordsMsg() const -> std::string {
    return config_->GetNoRecordsMessage();
}

void MonthTypFormatter::FormatPageSetup(std::stringstream& ss) const {
    ss << std::format(R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
        config_->GetMarginTopCm(),
        config_->GetMarginBottomCm(),
        config_->GetMarginLeftCm(),
        config_->GetMarginRightCm()
    ) << "\n";
}

void MonthTypFormatter::FormatHeaderContent(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{}", size: {}pt)[= {} {}])",
        config_->GetTitleFont(),
        config_->GetReportTitleFontSize(),
        config_->GetReportTitle(),
        data.year_month 
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->GetActualDaysLabel(), data.actual_days);
        ss << std::format("+ *{}:* {}\n", config_->GetTotalTimeLabel(), TimeFormatDuration(data.total_duration, data.actual_days));
    }
}

extern "C" {
    __declspec(dllexport) FormatterHandle CreateFormatter(const char* config_toml) { // NOLINT
        try {
            // [FIX] 使用 toml::parse
            auto config_tbl = toml::parse(config_toml);
            auto typ_config = std::make_shared<MonthTypConfig>(config_tbl);
            auto formatter = new MonthTypFormatter(typ_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) { // NOLINT
        if (handle) {
            delete static_cast<MonthTypFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* FormatReport(FormatterHandle handle, const MonthlyReportData& data) { // NOLINT
        if (handle) {
            auto* formatter = static_cast<MonthTypFormatter*>(handle);
            report_buffer = formatter->FormatReport(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
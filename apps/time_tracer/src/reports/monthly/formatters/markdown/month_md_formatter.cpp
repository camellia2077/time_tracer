// reports/monthly/formatters/markdown/month_md_formatter.cpp
#include "reports/monthly/formatters/markdown/month_md_formatter.hpp"
#include <format>
#include "reports/shared/utils/format/time_format.hpp"
#include <toml++/toml.h>

MonthMdFormatter::MonthMdFormatter(std::shared_ptr<MonthMdConfig> config) 
    : BaseMdFormatter(config) {}

auto MonthMdFormatter::ValidateData(const MonthlyReportData& data) const -> std::string {
    if (data.year_month == "INVALID") {
        return config_->GetInvalidFormatMessage();
    }
    return "";
}

auto MonthMdFormatter::IsEmptyData(const MonthlyReportData& data) const -> bool {
    return data.actual_days == 0;
}

auto MonthMdFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
    return data.actual_days;
}

auto MonthMdFormatter::GetNoRecordsMsg() const -> std::string {
    return config_->GetNoRecordsMessage();
}

void MonthMdFormatter::FormatHeaderContent(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", 
        config_->GetReportTitle(), 
        data.year_month 
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->GetActualDaysLabel(), data.actual_days);
        ss << std::format("- **{0}**: {1}\n", config_->GetTotalTimeLabel(), TimeFormatDuration(data.total_duration, data.actual_days));
    }
}

extern "C" {
    __declspec(dllexport) FormatterHandle CreateFormatter(const char* config_toml) { // NOLINT
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

    __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) { // NOLINT
        if (handle) {
            delete static_cast<MonthMdFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* FormatReport(FormatterHandle handle, const MonthlyReportData& data) { // NOLINT
        if (handle) {
            auto* formatter = static_cast<MonthMdFormatter*>(handle);
            report_buffer = formatter->FormatReport(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
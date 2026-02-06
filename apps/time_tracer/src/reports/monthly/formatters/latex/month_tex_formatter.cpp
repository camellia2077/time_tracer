// reports/monthly/formatters/latex/month_tex_formatter.cpp
#include "reports/monthly/formatters/latex/month_tex_formatter.hpp"
#include "reports/monthly/formatters/latex/month_tex_utils.hpp"
#include <toml++/toml.h>

MonthTexFormatter::MonthTexFormatter(std::shared_ptr<MonthTexConfig> config) 
    : BaseTexFormatter(config) {}

auto MonthTexFormatter::ValidateData(const MonthlyReportData& data) const -> std::string {
    if (data.year_month == "INVALID") {
        return config_->GetInvalidFormatMessage() + "\n";
    }
    return "";
}

auto MonthTexFormatter::IsEmptyData(const MonthlyReportData& data) const -> bool {
    return data.total_duration <= 0;
}

auto MonthTexFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
    return data.actual_days;
}

auto MonthTexFormatter::GetNoRecordsMsg() const -> std::string {
    return config_->GetNoRecordsMessage();
}

void MonthTexFormatter::FormatHeaderContent(std::stringstream& ss, const MonthlyReportData& data) const {
    MonthTexUtils::DisplayHeader(ss, data, config_);
}

extern "C" {
    __declspec(dllexport) FormatterHandle CreateFormatter(const char* config_toml) { // NOLINT
        try {
            // [FIX] 直接使用 parse 结果作为 table
            auto config_tbl = toml::parse(config_toml);
            
            // [FIX] 移除解引用
            auto tex_config = std::make_shared<MonthTexConfig>(config_tbl);
            auto formatter = new MonthTexFormatter(tex_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) { // NOLINT
        if (handle) {
            delete static_cast<MonthTexFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* FormatReport(FormatterHandle handle, const MonthlyReportData& data) { // NOLINT
        if (handle) {
            auto* formatter = static_cast<MonthTexFormatter*>(handle);
            report_buffer = formatter->FormatReport(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
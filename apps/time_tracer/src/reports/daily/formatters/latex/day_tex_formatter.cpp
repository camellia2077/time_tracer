// reports/daily/formatters/latex/day_tex_formatter.cpp
#include "reports/daily/formatters/latex/day_tex_formatter.hpp"
#include "reports/daily/formatters/latex/day_tex_utils.hpp"
#include "reports/daily/formatters/statistics/stat_formatter.hpp"
#include "reports/daily/formatters/statistics/latex_strategy.hpp"
#include <memory>
#include <toml++/toml.h>

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config)
    : BaseTexFormatter(config) {}

auto DayTexFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
    return data.total_duration == 0;
}

auto DayTexFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
    return 1;
}

auto DayTexFormatter::GetNoRecordsMsg() const -> std::string {
    return config_->GetNoRecords();
}

auto DayTexFormatter::GetKeywordColors() const
    -> std::map<std::string, std::string> {
    return config_->GetKeywordColors();
}

void DayTexFormatter::FormatHeaderContent(std::stringstream& report_stream,
                                            const DailyReportData& data) const {
    DayTexUtils::DisplayHeader(report_stream, data, config_);

}

void DayTexFormatter::FormatExtraContent(std::stringstream& report_stream,
                                           const DailyReportData& data) const {
    auto strategy = std::make_unique<LatexStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    report_stream << stats_formatter.Format(data, config_);
    DayTexUtils::DisplayDetailedActivities(report_stream, data, config_);

}

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

extern "C" {
    // [核心修改] 解析 TOML 字符串
    // Public API: keep symbol name stable for dynamic loading.
    __declspec(dllexport) auto CreateFormatter(const char* config_content) // NOLINT
        -> FormatterHandle {
        try {
            auto config_tbl = toml::parse(config_content);
            auto tex_config = std::make_shared<DayTexConfig>(config_tbl);
            auto formatter = std::make_unique<DayTexFormatter>(tex_config);
            return static_cast<FormatterHandle>(formatter.release());
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) { // NOLINT
        if (handle != nullptr) {
            std::unique_ptr<DayTexFormatter>{
                static_cast<DayTexFormatter*>(handle)};
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) auto FormatReport(FormatterHandle handle,
                                              const DailyReportData& data) // NOLINT
        -> const char* {
        if (handle != nullptr) {
            auto* formatter = static_cast<DayTexFormatter*>(handle);
            report_buffer = formatter->FormatReport(data);
            return report_buffer.c_str();
        }
        return kEmptyReport;
    }
}

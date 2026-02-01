// reports/daily/formatters/latex/day_tex_formatter.cpp
#include "day_tex_formatter.hpp"
#include "day_tex_utils.hpp"
#include "reports/daily/formatters/statistics/stat_formatter.hpp"
#include "reports/daily/formatters/statistics/latex_strategy.hpp"
#include <memory>
#include <toml++/toml.h>

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config)
    : BaseTexFormatter(config) {}

auto DayTexFormatter::is_empty_data(const DailyReportData& data) const -> bool {
    return data.total_duration == 0;
}

auto DayTexFormatter::get_avg_days(const DailyReportData& /*data*/) const -> int {
    return 1;
}

auto DayTexFormatter::get_no_records_msg() const -> std::string {
    return config_->get_no_records();
}

auto DayTexFormatter::get_keyword_colors() const
    -> std::map<std::string, std::string> {
    return config_->get_keyword_colors();
}

void DayTexFormatter::format_header_content(std::stringstream& report_stream,
                                            const DailyReportData& data) const {
    DayTexUtils::display_header(report_stream, data, config_);
}

void DayTexFormatter::format_extra_content(std::stringstream& report_stream,
                                           const DailyReportData& data) const {
    auto strategy = std::make_unique<LatexStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    report_stream << stats_formatter.format(data, config_);
    DayTexUtils::display_detailed_activities(report_stream, data, config_);
}

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

extern "C" {
    // [核心修改] 解析 TOML 字符串
    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) auto create_formatter(const char* config_content)
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

    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle != nullptr) {
            std::unique_ptr<DayTexFormatter>{
                static_cast<DayTexFormatter*>(handle)};
        }
    }

    static std::string report_buffer;

    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) auto format_report(FormatterHandle handle,
                                              const DailyReportData& data)
        -> const char* {
        if (handle != nullptr) {
            auto* formatter = static_cast<DayTexFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return kEmptyReport;
    }
}

// reports/daily/formatters/typst/day_typ_formatter.cpp
#include "day_typ_formatter.hpp"
#include "day_typ_utils.hpp"

#include "reports/daily/formatters/statistics/stat_formatter.hpp"
#include "reports/daily/formatters/statistics/typst_strategy.hpp"
#include <memory>
#include <toml++/toml.h> 

DayTypFormatter::DayTypFormatter(std::shared_ptr<DayTypConfig> config) 
    : BaseTypFormatter(config) {}

auto DayTypFormatter::is_empty_data(const DailyReportData& data) const -> bool {
    return data.total_duration == 0;
}

auto DayTypFormatter::get_avg_days(const DailyReportData& /*data*/) const -> int {
    return 1;
}

auto DayTypFormatter::get_no_records_msg() const -> std::string {
    return config_->get_no_records();
}

void DayTypFormatter::format_header_content(std::stringstream& report_stream, const DailyReportData& data) const {
    DayTypUtils::DisplayHeader(report_stream, data, config_);
}

void DayTypFormatter::format_extra_content(std::stringstream& report_stream, const DailyReportData& data) const {
    auto strategy = std::make_unique<TypstStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    report_stream << stats_formatter.format(data, config_);

    DayTypUtils::DisplayDetailedActivities(report_stream, data, config_);
}

extern "C" {
    // Required exports: create_formatter / destroy_formatter / format_report (lowercase)
    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) auto create_formatter(const char* config_content) -> FormatterHandle {
        try {
            auto config_tbl = toml::parse(config_content);
            auto typ_config = std::make_shared<DayTypConfig>(config_tbl);
            auto* formatter = new DayTypFormatter(typ_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle != nullptr) {
            delete static_cast<DayTypFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) auto format_report(FormatterHandle handle, const DailyReportData& data) -> const char* {
        if (handle != nullptr) {
            auto* formatter = static_cast<DayTypFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }

    // Backward-compatible exports (legacy symbol names)
    __declspec(dllexport) auto CreateFormatter(const char* config_content) -> FormatterHandle {
        return create_formatter(config_content);
    }

    __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {
        destroy_formatter(handle);
    }

    __declspec(dllexport) auto FormatReport(FormatterHandle handle, const DailyReportData& data) -> const char* {
        return format_report(handle, data);
    }
}

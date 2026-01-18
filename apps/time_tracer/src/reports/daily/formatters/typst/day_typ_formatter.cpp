// reports/daily/formatters/typst/day_typ_formatter.cpp
#include "day_typ_formatter.hpp"
#include "day_typ_utils.hpp"

#include "reports/daily/formatters/statistics/stat_formatter.hpp"
#include "reports/daily/formatters/statistics/typst_strategy.hpp"
#include <memory>
#include <toml++/toml.h> 

DayTypFormatter::DayTypFormatter(std::shared_ptr<DayTypConfig> config) 
    : BaseTypFormatter(config) {}

bool DayTypFormatter::is_empty_data(const DailyReportData& data) const {
    return data.total_duration == 0;
}

int DayTypFormatter::get_avg_days(const DailyReportData& /*data*/) const {
    return 1;
}

std::string DayTypFormatter::get_no_records_msg() const {
    return config_->get_no_records();
}

void DayTypFormatter::format_header_content(std::stringstream& ss, const DailyReportData& data) const {
    DayTypUtils::display_header(ss, data, config_);
}

void DayTypFormatter::format_extra_content(std::stringstream& ss, const DailyReportData& data) const {
    auto strategy = std::make_unique<TypstStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);

    DayTypUtils::display_detailed_activities(ss, data, config_);
}

extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_content) {
        try {
            auto config_tbl = toml::parse(config_content);
            auto typ_config = std::make_shared<DayTypConfig>(config_tbl);
            auto formatter = new DayTypFormatter(typ_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayTypFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayTypFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
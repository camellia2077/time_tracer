// reports/daily/formatters/latex/DayTexFormatter.cpp
#include "DayTexFormatter.hpp"
#include "DayTexUtils.hpp"
#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/LatexStrategy.hpp"
#include <memory>
#include <toml++/toml.h> // [修改]

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config) 
    : BaseTexFormatter(config) {}

bool DayTexFormatter::is_empty_data(const DailyReportData& data) const {
    return data.total_duration == 0;
}

int DayTexFormatter::get_avg_days(const DailyReportData& /*data*/) const {
    return 1;
}

std::string DayTexFormatter::get_no_records_msg() const {
    return config_->get_no_records(); 
}

std::map<std::string, std::string> DayTexFormatter::get_keyword_colors() const {
    return config_->get_keyword_colors();
}

void DayTexFormatter::format_header_content(std::stringstream& ss, const DailyReportData& data) const {
    DayTexUtils::display_header(ss, data, config_);
}

void DayTexFormatter::format_extra_content(std::stringstream& ss, const DailyReportData& data) const {
    auto strategy = std::make_unique<LatexStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);
    DayTexUtils::display_detailed_activities(ss, data, config_);
}

extern "C" {
    // [核心修改] 解析 TOML 字符串
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_content) {
        try {
            auto config_tbl = toml::parse(config_content);
            auto tex_config = std::make_shared<DayTexConfig>(config_tbl);
            auto formatter = new DayTexFormatter(tex_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayTexFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayTexFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
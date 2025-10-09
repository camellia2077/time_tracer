// reports/daily/formatters/typ/DayTyp.cpp
#include "DayTyp.hpp"
#include "DayTypUtils.hpp"
#include "reports/shared/utils/format/TypUtils.hpp"
#include <format>
#include <string>
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/TypstStrategy.hpp"
#include "common/AppConfig.hpp"

// 移除了静态注册逻辑


DayTyp::DayTyp(std::shared_ptr<DayTypConfig> config) : config_(config) {}

std::string DayTyp::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))",
        config_->get_base_font(),
        config_->get_base_font_size(),
        spacing_str) << "\n\n";

    DayTypUtils::display_header(ss, data, config_);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }

    auto strategy = std::make_unique<TypstStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);

    DayTypUtils::display_detailed_activities(ss, data, config_);

    ss << TypUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        1, // avg_days for daily report is 1
        config_->get_category_title_font(),
        config_->get_category_title_font_size()
    );

    return ss.str();
}

// [新增] C-style functions to be exported from the DLL
// These functions will be the entry points for the main application
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto typ_config = std::make_shared<DayTypConfig>(cfg.day_typ_config_path);
        auto formatter = new DayTyp(typ_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayTyp*>(handle);
        }
    }

    // A static buffer to hold the result of format_report
    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayTyp*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
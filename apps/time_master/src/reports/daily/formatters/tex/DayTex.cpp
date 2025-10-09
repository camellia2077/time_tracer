// reports/daily/formatters/tex/DayTex.cpp
#include "DayTex.hpp"
#include "DayTexUtils.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/LatexStrategy.hpp"
#include "common/AppConfig.hpp" // [新增] 为导出函数引入 AppConfig
#include <sstream>
#include <memory>

// 移除了静态注册逻辑


DayTex::DayTex(std::shared_ptr<DayTexConfig> config) : config_(config) {}

std::string DayTex::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(),
        config_->get_cjk_main_font(),
        config_->get_base_font_size(),
        config_->get_margin_in(),
        config_->get_keyword_colors()
    );

    DayTexUtils::display_header(ss, data, config_);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
    } else {
        auto strategy = std::make_unique<LatexStrategy>(config_);
        StatFormatter stats_formatter(std::move(strategy));
        ss << stats_formatter.format(data, config_);
        
        DayTexUtils::display_detailed_activities(ss, data, config_);
        
        ss << TexUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            1, // 日报的 avg_days 总是 1
            config_->get_category_title_font_size(),
            config_->get_list_top_sep_pt(),
            config_->get_list_item_sep_ex()
        );
    }

    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

// [新增] C-style functions to be exported from the DLL
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto tex_config = std::make_shared<DayTexConfig>(cfg.day_tex_config_path);
        auto formatter = new DayTex(tex_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayTex*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayTex*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
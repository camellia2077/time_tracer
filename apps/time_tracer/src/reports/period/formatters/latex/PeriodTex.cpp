// reports/period/formatters/latex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include "PeriodTexUtils.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/period/formatters/latex/PeriodTexConfig.hpp"
#include "reports/shared/model/PeriodReportData.hpp"
#include "common/AppConfig.hpp"
#include <sstream>
#include <memory>


PeriodTex::PeriodTex(std::shared_ptr<PeriodTexConfig> config) : config_(config) {}

std::string PeriodTex::format_report(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(),
        config_->get_cjk_main_font(),
        config_->get_base_font_size(),
        config_->get_margin_in()
    );
    
    PeriodTexUtils::display_summary(ss, data, config_);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        // [新增] 添加 Project Breakdown 统领性标题
        int title_size = config_->get_category_title_font_size();
        ss << "{";
        ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
        ss << "\\section*{" << TexUtils::escape_latex(config_->get_project_breakdown_label()) << "}";
        ss << "}\n\n";

        ss << TexUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            data.actual_days,
            config_->get_category_title_font_size(),
            config_->get_list_top_sep_pt(),
            config_->get_list_item_sep_ex()
        );
    }

    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto tex_config = std::make_shared<PeriodTexConfig>(cfg.period_tex_config_path);
        auto formatter = new PeriodTex(tex_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<PeriodTex*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const PeriodReportData& data) {
        if (handle) {
            auto* formatter = static_cast<PeriodTex*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}
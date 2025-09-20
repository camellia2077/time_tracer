// queries/period/formatters/tex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <format>
#include "queries/shared/utils/format/TimeFormat.hpp"    
#include "queries/shared/formatters/latex/TexUtils.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "queries/period/formatters/tex/PeriodTexConfig.hpp"    // [新增]
#include "queries/shared/data/PeriodReportData.hpp"            // [新增]

// [新增] 自我注册逻辑
namespace {
    struct PeriodTexRegister {
        PeriodTexRegister() {
            GenericFormatterFactory<PeriodReportData>::regist(ReportFormat::LaTeX, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
                    auto tex_config = std::make_shared<PeriodTexConfig>(cfg.period_tex_config_path);
                    return std::make_unique<PeriodTex>(tex_config);
                });
        }
    };
    const PeriodTexRegister registrar;
}


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
    
    _display_summary(ss, data);
    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        _display_project_breakdown(ss, data);
    }

    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    int title_size = config_->get_report_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
    ss << "\\section*{"
       << config_->get_report_title_prefix() << " " << data.days_to_query << " "
       << config_->get_report_title_days() << " ("
       << TexUtils::escape_latex(data.start_date) << " " << config_->get_report_title_date_separator() << " "
       << TexUtils::escape_latex(data.end_date) << ")}";
    ss << "}\n\n";

    if (data.actual_days > 0) {
        std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
            config_->get_list_top_sep_pt(), 
            config_->get_list_item_sep_ex()
        );
        ss << "\\begin{itemize}" << compact_list_options << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label() << "}: "
           << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: "
           << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    ss << TexUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        data.actual_days,
        config_->get_category_title_font_size(),
        config_->get_list_top_sep_pt(),
        config_->get_list_item_sep_ex()
    );
}
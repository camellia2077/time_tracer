// queries/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <format>
#include "queries/shared/utils/format/TimeFormat.hpp"     
#include "queries/shared/formatters/latex/TexUtils.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "queries/monthly/formatters/tex/MonthTexConfig.hpp"    // [新增]
#include "queries/shared/data/MonthlyReportData.hpp"         // [新增]

// [新增] 自我注册逻辑
namespace {
    struct MonthTexRegister {
        MonthTexRegister() {
            GenericFormatterFactory<MonthlyReportData>::regist(ReportFormat::LaTeX, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
                    auto tex_config = std::make_shared<MonthTexConfig>(cfg.month_tex_config_path);
                    return std::make_unique<MonthTex>(tex_config);
                });
        }
    };
    const MonthTexRegister registrar;
}


MonthTex::MonthTex(std::shared_ptr<MonthTexConfig> config) : config_(config) {}

std::string MonthTex::format_report(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
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

void MonthTex::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title_month = data.year_month.substr(0, 4) + "-" + data.year_month.substr(4, 2);
    
    int title_size = config_->get_report_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
    ss << "\\section*{" << config_->get_report_title() << " " << TexUtils::escape_latex(title_month) << "}";
    ss << "}\n\n";

    if (data.actual_days > 0) {
        std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
            config_->get_list_top_sep_pt(), 
            config_->get_list_item_sep_ex()
        );
        ss << "\\begin{itemize}" << compact_list_options << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label()  << "}: " << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void MonthTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << TexUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        data.actual_days,
        config_->get_category_title_font_size(),
        config_->get_list_top_sep_pt(),
        config_->get_list_item_sep_ex()
    );
}
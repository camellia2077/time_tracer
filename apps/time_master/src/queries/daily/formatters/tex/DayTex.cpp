#include "DayTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <format>
#include "queries/shared/utils/format/BoolToString.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/ReportStringUtils.hpp"
#include "queries/shared/formatters/latex/TexUtils.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "queries/daily/formatters/tex/DayTexConfig.hpp"
#include "queries/shared/data/DailyReportData.hpp"

// [新增] 自我注册逻辑
namespace {
    struct DayTexRegister {
        DayTexRegister() {
            GenericFormatterFactory<DailyReportData>::regist(ReportFormat::LaTeX, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<DailyReportData>> {
                    auto tex_config = std::make_shared<DayTexConfig>(cfg.day_tex_config_path);
                    return std::make_unique<DayTex>(tex_config);
                });
        }
    };
    const DayTexRegister registrar;
}

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

    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        _display_statistics(ss, data);
        _display_detailed_activities(ss, data);
        _display_project_breakdown(ss, data);
    }
    
    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

// ... (其余函数实现保持不变)
void DayTex::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    int title_size = config_->get_report_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
    ss << "\\section*{" << config_->get_report_title() << " " << TexUtils::escape_latex(data.date) << "}";
    ss << "}\n\n";
    
    std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
        config_->get_list_top_sep_pt(), 
        config_->get_list_item_sep_ex()
    );

    ss << "\\begin{itemize}" << compact_list_options << "\n";
    ss << "    \\item \\textbf{" << config_->get_date_label()      << "}: " << TexUtils::escape_latex(data.date) << "\n";
    ss << "    \\item \\textbf{" << config_->get_total_time_label() << "}: " << TexUtils::escape_latex(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_status_label()    << "}: " << TexUtils::escape_latex(bool_to_string(data.metadata.status)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_sleep_label()     << "}: " << TexUtils::escape_latex(bool_to_string(data.metadata.sleep)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_exercise_label()  << "}: " << TexUtils::escape_latex(bool_to_string(data.metadata.exercise)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_getup_time_label() << "}: " << TexUtils::escape_latex(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{" << config_->get_remark_label()    << "}: " << TexUtils::escape_latex(data.metadata.remark) << "\n";
    ss << "\\end{itemize}\n\n";
}

void DayTex::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const {
    ss << TexUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        1, // avg_days for daily report is 1
        config_->get_category_title_font_size(),
        config_->get_list_top_sep_pt(),
        config_->get_list_item_sep_ex()
    );
}

void DayTex::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    int category_size = config_->get_category_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << category_size << "}{" << category_size * 1.2 << "}\\selectfont";
    ss << "\\subsection*{" << config_->get_statistics_label() << "}";
    ss << "}\n\n";

    std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
        config_->get_list_top_sep_pt(), 
        config_->get_list_item_sep_ex()
    );
    ss << "\\begin{itemize}" << compact_list_options << "\n";
    ss << "    \\item \\textbf{" << config_->get_sleep_time_label() << "}: "
       << TexUtils::escape_latex(time_format_duration(data.sleep_time)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_anaerobic_time_label() << "}: "
       << TexUtils::escape_latex(time_format_duration(data.anaerobic_time)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_cardio_time_label() << "}: "
       << TexUtils::escape_latex(time_format_duration(data.cardio_time)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_grooming_time_label() << "}: "
       << TexUtils::escape_latex(time_format_duration(data.grooming_time)) << "\n";
    ss << "\\end{itemize}\n\n";
}

void DayTex::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (data.detailed_records.empty()) {
        return;
    }
    
    int category_size = config_->get_category_title_font_size();
    ss << "{";
    ss << "\\fontsize{" << category_size << "}{" << category_size * 1.2 << "}\\selectfont";
    ss << "\\subsection*{" << config_->get_all_activities_label() << "}";
    ss << "}\n\n";

    std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
        config_->get_list_top_sep_pt(), 
        config_->get_list_item_sep_ex()
    );
    ss << "\\begin{itemize}" << compact_list_options << "\n";

    for (const auto& record : data.detailed_records) {
        std::string project_path = replace_all(record.project_path, "_", config_->get_activity_connector());
        std::string base_string = TexUtils::escape_latex(record.start_time) + " - " +
                                  TexUtils::escape_latex(record.end_time) + " (" +
                                  TexUtils::escape_latex(time_format_duration(record.duration_seconds)) +
                                  "): " + TexUtils::escape_latex(project_path);
        
        std::string colorized_string = base_string;
        
        for (const auto& pair : config_->get_keyword_colors()) {
            if (record.project_path.find(pair.first) != std::string::npos) {
                colorized_string = "\\textcolor{" + pair.first + "color}{" + base_string + "}";
                break;
            }
        }
        
        ss << "    \\item " << colorized_string << "\n";

        if (record.activityRemark.has_value()) {
            ss << "    \\begin{itemize}" << compact_list_options << "\n";
            ss << "        \\item \\textbf{" << config_->get_activity_remark_label() << "}: "
               << TexUtils::escape_latex(record.activityRemark.value()) << "\n";
            ss << "    \\end{itemize}\n";
        }
    }
    ss << "\\end{itemize}\n\n";
}
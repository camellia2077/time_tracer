// reports/daily/formatters/tex/DayTex.cpp
#include "DayTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <format>
#include "reports/shared/utils/format/BoolToString.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/ReportStringUtils.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"

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
    const auto& items_config = config_->get_statistics_items();
    std::vector<std::string> lines_to_print;
    
    // [核心修改] 更新 ordered_keys
    const std::vector<std::string> ordered_keys = {"sleep_time", "total_exercise_time", "grooming_time", "recreation_time"};

    for (const auto& key : ordered_keys) {
        auto it = items_config.find(key);
        if (it == items_config.end() || !it->second.show) continue;

        long long duration = 0;
        if (key == "sleep_time") duration = data.sleep_time;
        else if (key == "total_exercise_time") duration = data.total_exercise_time;
        else if (key == "grooming_time") duration = data.grooming_time;
        else if (key == "recreation_time") duration = data.recreation_time;

        std::stringstream line_ss;
        line_ss << "    \\item \\textbf{" << it->second.label << "}: " << TexUtils::escape_latex(time_format_duration(duration));
        lines_to_print.push_back(line_ss.str());

        // [核心修改] 新增对 total_exercise_time 子项的处理
        if (key == "total_exercise_time") {
            std::vector<std::string> sub_exercise_lines;
            if (items_config.count("anaerobic_time") && items_config.at("anaerobic_time").show) {
                sub_exercise_lines.push_back(std::format("        \\item \\textbf{{{}}}: {}", items_config.at("anaerobic_time").label, TexUtils::escape_latex(time_format_duration(data.anaerobic_time))));
            }
            if (items_config.count("cardio_time") && items_config.at("cardio_time").show) {
                sub_exercise_lines.push_back(std::format("        \\item \\textbf{{{}}}: {}", items_config.at("cardio_time").label, TexUtils::escape_latex(time_format_duration(data.cardio_time))));
            }
            
            if (!sub_exercise_lines.empty()) {
                std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", config_->get_list_top_sep_pt(), config_->get_list_item_sep_ex());
                lines_to_print.push_back("    \\begin{itemize}" + compact_list_options);
                for (const auto& sub_line : sub_exercise_lines) {
                    lines_to_print.push_back(sub_line);
                }
                lines_to_print.push_back("    \\end{itemize}");
            }
        }

        if (key == "recreation_time") {
            std::vector<std::string> sub_recreation_lines;
            if (items_config.count("zhihu_time") && items_config.at("zhihu_time").show) {
                sub_recreation_lines.push_back(std::format("        \\item \\textbf{{{}}}: {}", items_config.at("zhihu_time").label, TexUtils::escape_latex(time_format_duration(data.recreation_zhihu_time))));
            }
            if (items_config.count("bilibili_time") && items_config.at("bilibili_time").show) {
                sub_recreation_lines.push_back(std::format("        \\item \\textbf{{{}}}: {}", items_config.at("bilibili_time").label, TexUtils::escape_latex(time_format_duration(data.recreation_bilibili_time))));
            }
            if (items_config.count("douyin_time") && items_config.at("douyin_time").show) {
                sub_recreation_lines.push_back(std::format("        \\item \\textbf{{{}}}: {}", items_config.at("douyin_time").label, TexUtils::escape_latex(time_format_duration(data.recreation_douyin_time))));
            }
            if (!sub_recreation_lines.empty()) {
                std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", config_->get_list_top_sep_pt(), config_->get_list_item_sep_ex());
                lines_to_print.push_back("    \\begin{itemize}" + compact_list_options);
                for (const auto& sub_line : sub_recreation_lines) {
                    lines_to_print.push_back(sub_line);
                }
                lines_to_print.push_back("    \\end{itemize}");
            }
        }
    }

    if (lines_to_print.empty()) {
        return;
    }

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
    for(const auto& line : lines_to_print) {
        ss << line << "\n";
    }
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
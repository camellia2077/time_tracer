// queries/daily/formatters/tex/DayTex.cpp
#include "DayTex.hpp"
#include "DayTexStrings.hpp" // 仍然用于颜色定义

#include <iomanip>
#include <string>
#include <sstream>

#include "common/utils/TimeUtils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/utils/TimeFormat.hpp" 

namespace {
    // 本地辅助函数，用于转义 TeX 特殊字符
    std::string escape_tex_local(const std::string& s) {
        std::string escaped;
        escaped.reserve(s.length());
        for (char c : s) {
            if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    }
}

// [修改] 实现新的构造函数
DayTex::DayTex(std::shared_ptr<DayTexConfig> config) : config_(config) {}

std::string DayTex::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << get_tex_preamble();

    _display_header(ss, data);

    if (data.total_duration == 0) {
        // [修改] 使用配置对象
        ss << config_->get_no_records_message() << "\n";
    } else {
        _display_statistics(ss, data);
        _display_detailed_activities(ss, data);
        _display_project_breakdown(ss, data, db);
    }
    
    ss << get_tex_postfix();
    return ss.str();
}

void DayTex::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    // [修改] 所有硬编码的文本都通过 config_ 对象获取
    ss << "\\section*{" << config_->get_report_title() << " " << escape_tex_local(data.date) << "}\n\n";
    
    ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
    ss << "    \\item \\textbf{" << config_->get_date_label()      << "}: " << escape_tex_local(data.date) << "\n";
    ss << "    \\item \\textbf{" << config_->get_total_time_label() << "}: " << escape_tex_local(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_status_label()    << "}: " << escape_tex_local(bool_to_string(data.metadata.status)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_sleep_label()     << "}: " << escape_tex_local(bool_to_string(data.metadata.sleep)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_exercise_label()  << "}: " << escape_tex_local(bool_to_string(data.metadata.exercise)) << "\n";
    ss << "    \\item \\textbf{" << config_->get_getup_time_label() << "}: " << escape_tex_local(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{" << config_->get_remark_label()    << "}: " << escape_tex_local(data.metadata.remark) << "\n";
    ss << "\\end{itemize}\n\n";
}

void DayTex::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX, 
        db, 
        data.records, 
        data.total_duration, 
        1 
    );
}

void DayTex::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    // [修改] 使用配置对象
    ss << "\\subsection*{" << config_->get_statistics_label() << "}\n\n";
    ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
    ss << "    \\item \\textbf{" << config_->get_sleep_time_label() << "}: "
       << escape_tex_local(time_format_duration_hm(data.sleep_time)) << "\n";
    ss << "\\end{itemize}\n\n";
}

void DayTex::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (data.detailed_records.empty()) {
        return;
    }
    
    ss << "\\subsection*{" << config_->get_all_activities_label() << "}\n\n";
    ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";

    for (const auto& record : data.detailed_records) {
        std::string base_string = escape_tex_local(record.start_time) + " - " +
                                  escape_tex_local(record.end_time) + " (" +
                                  escape_tex_local(time_format_duration_hm(record.duration_seconds)) +
                                  "): " + escape_tex_local(record.project_path);
        
        std::string colorized_string = base_string;

        // [修改] 适应新的 DayTexStrings::KeywordColors 结构
        for (const auto& pair : DayTexStrings::KeywordColors) {
            if (record.project_path.find(pair.first) != std::string::npos) {
                // 从 ColorInfo 结构中获取颜色名称 (pair.second.name)
                colorized_string = "\\textcolor{" + pair.second.name + "}{" + base_string + "}";
                break;
            }
        }
        
        ss << "    \\item " << colorized_string << "\n";

        if (record.activityRemark.has_value()) {
            ss << "    \\begin{itemize}" << config_->get_compact_list_options() << "\n";
            ss << "        \\item \\textbf{" << config_->get_activity_remark_label() << "}: " 
               << escape_tex_local(record.activityRemark.value()) << "\n";
            ss << "    \\end{itemize}\n";
        }
    }
    ss << "\\end{itemize}\n\n";
}

// [新增] 将模板代码直接添加到类实现中
std::string DayTex::get_tex_preamble() const {
    std::stringstream ss;
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage[dvipsnames]{xcolor}\n";
    ss << "\\usepackage{enumitem}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";

    // [修改] 动态生成颜色定义
    // 遍历在 DayTexStrings.hpp 中定义的 KeywordColors map
    for (const auto& pair : DayTexStrings::KeywordColors) {
        const auto& color_info = pair.second;
        ss << "\\definecolor{" << color_info.name << "}{HTML}{" << color_info.hex << "}\n";
    }
    
    ss << "\n";
    // [修改] 使用配置中的字体
    ss << "\\setmainfont{" << config_->get_main_font() << "}\n";
    ss << "\\setCJKmainfont{" << config_->get_cjk_main_font() << "}\n\n";
    ss << "\\begin{document}\n\n";
    return ss.str();
}

std::string DayTex::get_tex_postfix() const {
    return "\n\\end{document}\n";
}
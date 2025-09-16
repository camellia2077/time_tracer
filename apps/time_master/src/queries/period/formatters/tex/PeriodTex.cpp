// queries/period/formatters/tex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "common/utils/TimeUtils.hpp"

namespace {
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

PeriodTex::PeriodTex(std::shared_ptr<PeriodTexConfig> config) : config_(config) {}

std::string PeriodTex::format_report(const PeriodReportData& data, sqlite3* db) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message() + "\n";
    }

    std::stringstream ss;
    ss << get_tex_preamble();
    
    _display_summary(ss, data);
    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }

    ss << get_tex_postfix();
    return ss.str();
}

void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "\\section*{"
       << config_->get_report_title_prefix() << " " << data.days_to_query << " "
       << config_->get_report_title_days() << " ("
       << escape_tex_local(data.start_date) << " " << config_->get_report_title_date_separator() << " "
       << escape_tex_local(data.end_date) << ")}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label() << "}: " 
           << escape_tex_local(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: " 
           << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* /*db*/) const {
    // [核心修改] 移除 db 参数
    ss << generate_project_breakdown(
        ReportFormat::LaTeX,
        data.records,
        data.total_duration,
        data.actual_days
    );
}

std::string PeriodTex::get_tex_preamble() const {
    std::stringstream ss;
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage[dvipsnames]{xcolor}\n";
    ss << "\\usepackage{enumitem}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n\n";
    ss << "\\setmainfont{" << config_->get_main_font() << "}\n";
    ss << "\\setCJKmainfont{" << config_->get_cjk_main_font() << "}\n\n";
    ss << "\\begin{document}\n\n";
    return ss.str();
}

std::string PeriodTex::get_tex_postfix() const {
    return "\n\\end{document}\n";
}
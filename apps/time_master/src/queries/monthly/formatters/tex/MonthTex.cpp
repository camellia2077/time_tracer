// queries/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"

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

MonthTex::MonthTex(std::shared_ptr<MonthTexConfig> config) : config_(config) {}

std::string MonthTex::format_report(const MonthlyReportData& data, sqlite3* db) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
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

void MonthTex::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title_month = data.year_month.substr(0, 4) + "-" + data.year_month.substr(4, 2);
    ss << "\\section*{" << config_->get_report_title() << " " << escape_tex_local(title_month) << "}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label()  << "}: " << escape_tex_local(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void MonthTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}

std::string MonthTex::get_tex_preamble() const {
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

std::string MonthTex::get_tex_postfix() const {
    return "\n\\end{document}\n";
}
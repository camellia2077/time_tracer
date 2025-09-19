// queries/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>

#include "queries/shared/utils/db/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/interface/ITreeFmt.hpp"
#include "common/utils/TimeUtils.hpp"
#include "queries/shared/utils/tex/TexUtils.hpp"

MonthTex::MonthTex(std::shared_ptr<MonthTexConfig> config) : config_(config) {}

std::string MonthTex::format_report(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(config_->get_main_font(), config_->get_cjk_main_font());

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
    ss << "\\section*{" << config_->get_report_title() << " " << TexUtils::escape_latex(title_month) << "}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label()  << "}: " << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void MonthTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX,
        data.records,
        data.total_duration,
        data.actual_days
    );
}
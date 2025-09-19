// queries/period/formatters/tex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>

#include "queries/shared/utils/db/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "common/utils/TimeUtils.hpp"
#include "queries/shared/utils/tex/TexUtils.hpp"

PeriodTex::PeriodTex(std::shared_ptr<PeriodTexConfig> config) : config_(config) {}

std::string PeriodTex::format_report(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message() + "\n";
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

void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "\\section*{"
       << config_->get_report_title_prefix() << " " << data.days_to_query << " "
       << config_->get_report_title_days() << " ("
       << TexUtils::escape_latex(data.start_date) << " " << config_->get_report_title_date_separator() << " "
       << TexUtils::escape_latex(data.end_date) << ")}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label() << "}: "
           << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: "
           << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX,
        data.records,
        data.total_duration,
        data.actual_days
    );
}
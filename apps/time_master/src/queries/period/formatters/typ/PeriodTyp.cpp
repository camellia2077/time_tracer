// queries/period/formatters/typ/PeriodTyp.cpp
#include "PeriodTyp.hpp"
#include <iomanip>
#include <format>
#include "common/utils/ProjectTree.hpp"
#include "queries/shared/utils/report/ReportDataUtils.hpp" 
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"     


PeriodTyp::PeriodTyp(std::shared_ptr<PeriodTypConfig> config) : config_(config) {}

std::string PeriodTyp::format_report(const PeriodReportData& data) const {
    std::stringstream ss;
    
    ss << std::format(R"(#set text(font: "{0}"))", config_->get_content_font()) << "\n\n";

    if (data.days_to_query <= 0) {
        ss << config_->get_positive_days_error() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data);
    return ss.str();
}

void PeriodTyp::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3} days ({4} to {5})])",
        config_->get_title_font(),
        config_->get_title_font_size(),
        config_->get_title_prefix(),
        data.days_to_query,
        data.start_date,
        data.end_date
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{0}:* {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("+ *{0}:* {1}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    // [核心修改] 移除 db 参数
    ss << generate_project_breakdown(
        ReportFormat::Typ,
        data.records,
        data.total_duration,
        data.actual_days
    );
}
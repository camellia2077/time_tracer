// queries/period/formatters/md/PeriodMd.cpp
#include "PeriodMd.hpp"
#include <iomanip>
#include <format>
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/interfaces/ITreeFmt.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"

PeriodMd::PeriodMd(std::shared_ptr<PeriodMdConfig> config) : config_(config) {}

std::string PeriodMd::format_report(const PeriodReportData& data) const {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        ss << config_->get_invalid_days_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data);
    return ss.str();
}

void PeriodMd::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << std::format("## {0} {1} {2} ({3} {4} {5})\n\n",
        config_->get_report_title_prefix(),
        data.days_to_query,
        config_->get_report_title_days(),
        data.start_date,
        config_->get_report_title_date_separator(),
        data.end_date
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodMd::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    // [核心修改] 直接使用 data 中的 project_tree 进行格式化
    auto formatter = TreeFmtFactory::createFormatter(ReportFormat::Markdown);
    if (formatter) {
        ss << formatter->format(data.project_tree, data.total_duration, data.actual_days);
    }
}
// queries/monthly/formatters/md/MonthMd.cpp
#include "MonthMd.hpp"
#include <iomanip>
#include <format>

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "common/utils/ProjectTree.hpp"

MonthMd::MonthMd(std::shared_ptr<MonthMdConfig> config) : config_(config) {}

std::string MonthMd::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        ss << config_->get_invalid_format_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthMd::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << std::format("## {0} {1}-{2}\n\n", 
        config_->get_report_title(), 
        data.year_month.substr(0, 4), 
        data.year_month.substr(4, 2)
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

void MonthMd::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Markdown,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}
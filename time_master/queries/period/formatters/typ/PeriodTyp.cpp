#include "common/pch.h"
#include "PeriodTyp.h"
#include <iomanip>

#include "common/common_utils.h"
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"

std::string PeriodTyp::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << "#set text(font: \"Noto Serif SC\")\n\n";
    if (data.days_to_query <= 0) {
        ss << "Number of days to query must be positive.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void PeriodTyp::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "= Period Report: Last " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ") \n\n";

    if (data.actual_days > 0) {
        ss << "+ *Total Time Recorded*: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "+ *Actual Days with Records*: " << data.actual_days << "\n";
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    auto formatter = TreeFmtFactory::createFormatter(ReportFormat::Typ);

    if (formatter) {
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, data.actual_days);
        ss << breakdown_output;
    }
}
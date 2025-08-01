#include "common/pch.h"
#include "MonthTyp.h"
#include <iomanip>

#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "common/common_utils.h"

std::string MonthTyp::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << "#set text(font: \"Noto Serif SC\")\n\n";
    if (data.year_month == "INVALID") {
        ss << "Invalid year_month format. Expected YYYYMM.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthTyp::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << "= Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << " \n\n";

    if (data.actual_days > 0) {
        ss << "+ *Actual Days with Records*: " << data.actual_days << "\n";
        ss << "+ *Total Time Recorded*: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
}

void MonthTyp::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    auto formatter = TreeFmtFactory::createFormatter(ReportFormat::Typ);

    if (formatter) {
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, data.actual_days);
        ss << breakdown_output;
    }
}
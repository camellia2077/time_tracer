#include "common/pch.h"
#include "DayTyp.h"
#include <iomanip>

#include "common/common_utils.h"
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "queries/shared/DailyReportData.h"

std::string DayTyp::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << "#set text(font: \"Noto Serif SC\")\n\n";
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DayTyp::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "= Daily Report for " << data.date << "\n\n";
    ss << "+ *Date*: " << data.date << "\n";
    ss << "+ *Total Time Recorded*: " << time_format_duration(data.total_duration) << "\n";
    ss << "+ *Status*: " << data.metadata.status << "\n";
    ss << "+ *Getup Time*: " << data.metadata.getup_time << "\n";
    ss << "+ *Remark*:" << data.metadata.remark << "\n";
}

void DayTyp::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    auto formatter = TreeFmtFactory::createFormatter(ReportFormat::Typ);

    if (formatter) {
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, 1);
        ss << breakdown_output;
    }
}
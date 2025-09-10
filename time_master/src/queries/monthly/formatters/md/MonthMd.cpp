// queries/monthly/formatters/MonthMd.cpp
#include "MonthMd.hpp"
#include "MonthMdConfig.hpp" // [MODIFIED] Include the new configuration file

#include <iomanip>

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "common/common_utils.hpp"

std::string MonthMd::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        // [MODIFIED] Use the configurable error message
        ss << MonthMdConfig::InvalidFormatMessage << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        // [MODIFIED] Use the configurable message
        ss << MonthMdConfig::NoRecordsMessage << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthMd::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    // [MODIFIED] Markdown syntax is now hardcoded here and combined with labels from the config file.
    ss << "## " << MonthMdConfig::ReportTitle << " " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << "\n\n";

    if (data.actual_days > 0) {
        ss << "- **" << MonthMdConfig::ActualDaysLabel << "**: " << data.actual_days << "\n";
        ss << "- **" << MonthMdConfig::TotalTimeLabel  << "**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
}

void MonthMd::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    // This function remains unchanged as it calls a separate utility.
    ss << generate_project_breakdown(
        ReportFormat::Markdown,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}
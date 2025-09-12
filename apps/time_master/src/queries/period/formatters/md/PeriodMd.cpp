// queries/report_generators/period/formatter/period_md/PeriodMd.cpp
#include "PeriodMd.hpp"
#include "PeriodMdConfig.hpp" // [MODIFIED] Include the new configuration file

#include <iomanip>

#include "common/common_utils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"

std::string PeriodMd::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        // [MODIFIED] Use the configurable error message
        ss << PeriodMdConfig::InvalidDaysMessage << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        // [MODIFIED] Use the configurable message
        ss << PeriodMdConfig::NoRecordsMessage << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void PeriodMd::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    // [MODIFIED] Markdown syntax is now hardcoded here and combined with labels from the config file.
    
    // Title: Reconstructed from configurable parts
    ss << "## "
       << PeriodMdConfig::ReportTitlePrefix << " " << data.days_to_query << " "
       << PeriodMdConfig::ReportTitleDays << " ("
       << data.start_date << " " << PeriodMdConfig::ReportTitleDateSeparator << " "
       << data.end_date << ")\n\n";

    if (data.actual_days > 0) {
        ss << "- **" << PeriodMdConfig::TotalTimeLabel << "**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "- **" << PeriodMdConfig::ActualDaysLabel << "**: " << data.actual_days << "\n";
    }
}

void PeriodMd::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    // This function remains unchanged as it calls a separate utility.
    ss << generate_project_breakdown(
        ReportFormat::Markdown,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}
// domain/reports/models/query_data_structs.hpp
#ifndef REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_H_
#define REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_H_

#include <map>
#include <string>
#include <vector>

struct FormattedDailyReportEntry {
  std::string report_id;
  std::string content;
};

using FormattedDailyReportsByMonth =
    std::map<int, std::vector<FormattedDailyReportEntry>>;
using FormattedGroupedReports = std::map<int, FormattedDailyReportsByMonth>;

using FormattedReportsByMonth = std::map<int, std::string>;
using FormattedMonthlyReports = std::map<int, FormattedReportsByMonth>;

using FormattedPeriodReports = std::map<int, std::string>;

using FormattedReportsByIsoWeek = std::map<int, std::string>;
using FormattedWeeklyReports = std::map<int, FormattedReportsByIsoWeek>;

using FormattedYearlyReports = std::map<int, std::string>;

#endif  // REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_H_

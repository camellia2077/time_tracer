// queries/period/formatters/tex/PeriodTexConfig.hpp
#ifndef PERIOD_REPORT_TEX_CONFIG_HPP
#define PERIOD_REPORT_TEX_CONFIG_HPP

#include <string_view>

namespace PeriodTexConfig {
    constexpr std::string_view ReportTitlePrefix        = "Period Report: Last";
    constexpr std::string_view ReportTitleDays          = "days";
    constexpr std::string_view ReportTitleDateSeparator = "to";
    constexpr std::string_view TotalTimeLabel  = "Total Time Recorded";
    constexpr std::string_view ActualDaysLabel = "Actual Days with Records";
    constexpr std::string_view NoRecordsMessage   = "No time records found in this period.";
    constexpr std::string_view InvalidDaysMessage = "Number of days to query must be positive.";
    // --- [核心修改] ---
    constexpr std::string_view CompactListOptions = "[topsep=0pt, itemsep=-0.5ex]";
}

#endif // PERIOD_REPORT_TEX_CONFIG_HPP
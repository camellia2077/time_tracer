// queries/monthly/formatters/tex/MonthTexConfig.hpp
#ifndef MONTHLY_REPORT_TEX_CONFIG_HPP
#define MONTHLY_REPORT_TEX_CONFIG_HPP

#include <string_view>

namespace MonthTexConfig {
    constexpr std::string_view ReportTitle = "Monthly Summary for";
    constexpr std::string_view ActualDaysLabel = "Actual Days with Records";
    constexpr std::string_view TotalTimeLabel  = "Total Time Recorded";
    constexpr std::string_view NoRecordsMessage = "No time records found for this month.";
    constexpr std::string_view InvalidFormatMessage = "Invalid year_month format. Expected YYYYMM.";
    // --- [核心修改] ---
    constexpr std::string_view CompactListOptions = "[topsep=0pt, itemsep=-0.5ex]";
} 

#endif // MONTHLY_REPORT_TEX_CONFIG_HPP
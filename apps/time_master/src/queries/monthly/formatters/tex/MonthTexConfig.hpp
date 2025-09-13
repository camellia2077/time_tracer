// queries/monthly/formatters/tex/MonthTexConfig.hpp
#ifndef MONTHLY_REPORT_TEX_CONFIG_HPP
#define MONTHLY_REPORT_TEX_CONFIG_HPP

#include <string_view>

/**
 * @brief Namespace containing user-configurable text labels for the monthly report.
 * A user can edit these strings (e.g., for translation) without needing to
 * understand TeX syntax or C++ code.
 */
namespace MonthTexConfig {

    // --- Report Header Text ---
    constexpr std::string_view ReportTitle = "Monthly Summary for";

    // --- Summary Item Labels ---
    constexpr std::string_view ActualDaysLabel = "Actual Days with Records";
    constexpr std::string_view TotalTimeLabel  = "Total Time Recorded";

    // --- Body Content & Error Messages ---
    constexpr std::string_view NoRecordsMessage = "No time records found for this month.";
    constexpr std::string_view InvalidFormatMessage = "Invalid year_month format. Expected YYYYMM.";

} // namespace MonthTexConfig

#endif // MONTHLY_REPORT_TEX_CONFIG_HPP
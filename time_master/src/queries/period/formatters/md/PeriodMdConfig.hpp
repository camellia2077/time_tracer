// queries/report_generators/period/formatter/period_md/PeriodMdConfig.hpp
#ifndef PERIOD_REPORT_MARKDOWN_CONFIG_HPP
#define PERIOD_REPORT_MARKDOWN_CONFIG_HPP

#include <string_view>

/**
 * @brief Namespace containing user-configurable text labels for the period Markdown report.
 * A user can edit these strings (e.g., for translation) without needing to
 * understand Markdown syntax or C++ code.
 */
namespace PeriodMdConfig {

    // --- Report Title Components ---
    constexpr std::string_view ReportTitlePrefix        = "Period Report: Last";
    constexpr std::string_view ReportTitleDays          = "days";
    constexpr std::string_view ReportTitleDateSeparator = "to";

    // --- Summary Item Labels ---
    constexpr std::string_view TotalTimeLabel  = "Total Time Recorded";
    constexpr std::string_view ActualDaysLabel = "Actual Days with Records";

    // --- Body Content & Error Messages ---
    constexpr std::string_view NoRecordsMessage   = "No time records found in this period.";
    constexpr std::string_view InvalidDaysMessage = "Number of days to query must be positive.";

} // namespace PeriodMdConfig

#endif // PERIOD_REPORT_MARKDOWN_CONFIG_HPP
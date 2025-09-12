// queries/daily/formatters/tex/DayTexStrings.hpp
#ifndef DAILY_REPORT_TEX_CONFIG_HPP
#define DAILY_REPORT_TEX_CONFIG_HPP

#include <string_view>

/**
 * @brief Namespace containing user-configurable text labels for the daily report.
 * A user can edit these strings (e.g., for translation into another language)
 * without needing to understand TeX syntax or C++ code.
 */
namespace DayTexConfig {

    // --- Report Header Text ---
    constexpr std::string_view ReportTitle = "Daily Report for";

    // --- Metadata Item Labels ---
    constexpr std::string_view DateLabel      = "Date";
    constexpr std::string_view TotalTimeLabel = "Total Time Recorded";
    constexpr std::string_view StatusLabel    = "Status";
    constexpr std::string_view SleepLabel     = "Sleep";
    constexpr std::string_view GetupTimeLabel = "Getup Time";
    constexpr std::string_view RemarkLabel    = "Remark";

    // --- Body Content Text ---
    constexpr std::string_view NoRecordsMessage = "No time records for this day.";
    // [新增] 新增的标签
    constexpr std::string_view StatisticsLabel = "Statistics";
    constexpr std::string_view AllActivitiesLabel = "All Activities";
    constexpr std::string_view SleepTimeLabel = "Sleep Time";
    constexpr std::string_view ActivityRemarkLabel = "Activity Remark"; // [新增] 活动备注的标

} // namespace DayTexConfig

#endif // DAILY_REPORT_TEX_CONFIG_HPP
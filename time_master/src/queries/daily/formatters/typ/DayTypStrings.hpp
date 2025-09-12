// queries/daily/formatters/typ/DayTypStrings.hpp
#ifndef DAY_TYP_STRINGS_HPP
#define DAY_TYP_STRINGS_HPP

#include <string>
#include <map>

namespace DayTypStrings {
    // （フォント、カラー設定は変更なし）
    const std::string TitleFont = "Noto Serif";
    const std::string ContentFont = "Noto Serif";
    const int TitleFontSize = 14;
    const std::map<std::string, std::string> KeywordColors = {
        {"study",      "rgb(\"#2ECC40\")"},
        {"recreation", "rgb(\"#FF4136\")"},
        {"meal",       "rgb(\"#FF851B\")"},
        {"exercise",   "rgb(\"#0074D9\")"},
        {"routine",    "rgb(\"#AAAAAA\")"},
        {"sleep",      "rgb(\"#B10DC9\")"},
        {"code",       "rgb(\"#39CCCC\")"}
    };

    // --- テキスト設定 (Text Configuration) ---
    const std::string TitlePrefix     = "Daily Report for";
    const std::string DateLabel       = "Date";
    const std::string TotalTimeLabel  = "Total Hours";
    const std::string StatusLabel     = "Status";
    const std::string SleepLabel      = "Sleep";
    const std::string GetupTimeLabel  = "Getup Time";
    const std::string RemarkLabel     = "Remark";
    const std::string NoRecords       = "No time records for this day.";
    const std::string StatisticsLabel = "Statistics";
    const std::string AllActivitiesLabel = "All Activities";
    const std::string SleepTimeLabel = "Sleep Time";
    const std::string ActivityRemarkLabel = "Activity Remark";

}

#endif // DAY_TYP_STRINGS_HPP
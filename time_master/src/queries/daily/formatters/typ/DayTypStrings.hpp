// queries/daily/formatters/typ/DayTypStrings.hpp
#ifndef DAY_TYP_STRINGS_HPP
#define DAY_TYP_STRINGS_HPP

#include <string>
#include <map>

namespace DayTypStrings {
    // 定义报告主标题使用的字体
    const std::string TitleFont = "Noto Serif";
    // 定义报告正文内容使用的字体
    const std::string ContentFont = "Noto Serif";

    // 定义报告主标题的字体大小
    const int TitleFontSize = 14;

    // --- [新增] 活动关键词颜色配置 ---
    const std::map<std::string, std::string> KeywordColors = {
        {"study",      "rgb(\"#2ECC40\")"},
        {"recreation", "rgb(\"#FF4136\")"},
        {"meal",       "rgb(\"#FF851B\")"},
        {"exercise",   "rgb(\"#0074D9\")"},
        {"routine",    "rgb(\"#AAAAAA\")"},
        {"sleep",      "rgb(\"#B10DC9\")"},
        {"code",       "rgb(\"#39CCCC\")"}
    }; 

    // --- 文本配置 (Text Configuration) ---
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

}

#endif // DAY_TYP_STRINGS_HPP
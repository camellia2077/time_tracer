// queries/daily/formatters/typ/DayTypStrings.hpp
#ifndef DAY_TYP_STRINGS_HPP
#define DAY_TYP_STRINGS_HPP

#include <string>

namespace DayTypStrings {
    // 定义报告主标题使用的字体
    const std::string TitleFont = "Noto Serif";
    // 定义报告正文内容使用的字体
    const std::string ContentFont = "Noto Serif";

    // 定义报告主标题的字体大小
    const int TitleFontSize = 14;

    // --- 文本配置 (Text Configuration) ---
    const std::string TitlePrefix     = "Daily Report for";
    const std::string DateLabel       = "Date";
    const std::string TotalTimeLabel  = "Total Hours"; 
    const std::string StatusLabel     = "Status";
    const std::string SleepLabel      = "Sleep"; //  Sleep 标签
    const std::string GetupTimeLabel  = "Getup Time";
    const std::string RemarkLabel     = "Remark";
    const std::string NoRecords       = "No time records for this day.";
    const std::string StatisticsLabel = "Statistics"; // 统计部分标题
    const std::string AllActivitiesLabel = "All Activities"; // 所有活动部分标题
    const std::string SleepTimeLabel = "Sleep Time"; // 睡眠时长标签

}

#endif // DAY_TYP_STRINGS_HPP
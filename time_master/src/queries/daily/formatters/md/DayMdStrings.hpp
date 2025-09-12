// queries/daily/formatters/md/DayMdStrings.hpp
#ifndef DAY_MD_STRINGS_HPP
#define DAY_MD_STRINGS_HPP

#include <string>

// =================================================================
//           DayMd 报告 UI 文本配置
// =================================================================

namespace DayMdStrings {

    const std::string TitlePrefix     = "Daily Report for";
    const std::string DateLabel       = "Date";
    const std::string TotalTimeLabel  = "Total Time Recorded";
    const std::string StatusLabel     = "Status";
    const std::string SleepLabel      = "Sleep";
    const std::string GetupTimeLabel  = "Getup Time";
    const std::string RemarkLabel     = "Remark";
    const std::string NoRecords       = "No time records for this day.";
    const std::string StatisticsLabel = "Statistics"; // [新增] 统计部分标题
    const std::string SleepTimeLabel  = "Sleep Time";   // [新增] 睡眠时长标签
    const std::string ActivityRemarkLabel = "Activety Remark"; // 新增：活动备注的标签
}

#endif // DAY_MD_STRINGS_HPP
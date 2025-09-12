// queries/daily/formatters/md/DayMdStrings.hpp
#ifndef DAY_MD_STRINGS_HPP
#define DAY_MD_STRINGS_HPP

#include <string>

// =================================================================
//           DayMd 報告 UI テキスト設定
// =================================================================

namespace DayMdStrings {

    const std::string TitlePrefix     = "Daily Report for";
    const std::string DateLabel       = "Date";
    const std::string TotalTimeLabel  = "Total Time Recorded";
    const std::string StatusLabel     = "Status";
    const std::string SleepLabel      = "Sleep";
    const std::string GetupTimeLabel  = "Getup Time";
    const std::string RemarkLabel     = "Remark";
    // --- [核心修改] 新增 Exercise 标签 ---
    const std::string ExerciseLabel   = "Exercise";
    const std::string NoRecords       = "No time records for this day.";
    const std::string StatisticsLabel = "Statistics";
    const std::string SleepTimeLabel  = "Sleep Time";  
    const std::string ActivityRemarkLabel = "Activity Remark"; 

}

#endif // DAY_MD_STRINGS_HPP
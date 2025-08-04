// queries/daily/formatters/typ/DayTypStrings.h
#ifndef DAY_TYP_STRINGS_H
#define DAY_TYP_STRINGS_H

#include <string>

// =================================================================
//           DayTyp 报告 UI 文本及样式配置
// =================================================================

namespace DayTypStrings {

    // --- 样式配置 (Style Configuration) ---
    /**
     * @brief 定义报告主标题使用的字体。
     */
    const std::string TitleFont = "Noto Serif SC";

    /**
     * @brief 定义报告正文内容使用的字体。
     * 这里特意选用了一个差别很大的字体以便于测试。
     */
    const std::string ContentFont = "Noto Serif SC";
    // --- 文本配置 (Text Configuration) ---
    const std::string TitlePrefix     = "Daily Report for";
    const std::string DateLabel       = "Date";
    const std::string TotalTimeLabel  = "Total Hours"; 
    const std::string StatusLabel     = "Status";
    const std::string GetupTimeLabel  = "Getup Time";
    const std::string RemarkLabel     = "Remark";
    const std::string NoRecords       = "No time records for this day.";

}

#endif // DAY_TYP_STRINGS_H
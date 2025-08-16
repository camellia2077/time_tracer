// queries/monthly/formatters/typ/MonthTypStrings.h
#ifndef MONTH_TYP_STRINGS_H
#define MONTH_TYP_STRINGS_H

#include <string>

// =================================================================
//           MonthTyp 报告 UI 文本及样式配置
// =================================================================

namespace MonthTypStrings {

    // --- 样式配置 (Style Configuration) ---
    /**
     * @brief 定义报告使用的全局正文字体。
     */
    const std::string BodyFont = "Noto Serif";

    /**
     * @brief 定义报告主标题使用的字体 (如果想和正文区分)。
     * 如果想和正文使用相同字体，只需将值设为和 BodyFont 一样即可。
     */
    const std::string TitleFont = "Noto Serif";

    /**
     * @brief 定义报告主标题的字体大小。(新增)
     */
    const int TitleFontSize = 14;


    // --- 文本配置 (Text Configuration) ---
    const std::string TitlePrefix         = "Monthly Summary for";
    const std::string ActualDaysLabel     = "Actual Days with Records";
    const std::string TotalTimeLabel      = "Total Hours";
    const std::string InvalidFormatError  = "Invalid year_month format. Expected YYYYMM.";
    const std::string NoRecords           = "No time records found for this month.";

}

#endif // MONTH_TYP_STRINGS_H
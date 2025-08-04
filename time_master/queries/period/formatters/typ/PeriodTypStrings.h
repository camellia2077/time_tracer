// queries/period/formatters/typ/PeriodTypStrings.h
#ifndef PERIOD_TYP_STRINGS_H
#define PERIOD_TYP_STRINGS_H

#include <string>

// =================================================================
//           PeriodTyp 报告 UI 文本及样式配置
// =================================================================

namespace PeriodTypStrings {

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
    const std::string TitlePrefix         = "Period Report: Last";
    const std::string TotalTimeLabel      = "Total Time Recorded";
    const std::string ActualDaysLabel     = "Actual Days with Records";
    const std::string PositiveDaysError   = "Number of days to query must be positive.";
    const std::string NoRecords           = "No time records found in this period.";
}

#endif // PERIOD_TYP_STRINGS_H
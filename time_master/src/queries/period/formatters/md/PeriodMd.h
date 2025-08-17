// PeriodMd.h
#ifndef PERIOD_REPORT_MARKDOWN_FORMATTER_H
#define PERIOD_REPORT_MARKDOWN_FORMATTER_H

#include "queries/shared/Interface/IReportFormatter.h"  // 替换 IPeriodFmt.h
#include "queries/shared/PeriodReportData.h"  // 为模板类型引入定义
#include <sstream>

// Forward declaration
struct PeriodReportData;

/**
 * @class PeriodMd
 * @brief 将周期报告数据格式化为 Markdown 字符串的具体实现。
 */
class PeriodMd : public IReportFormatter<PeriodReportData> { // 继承自模板化通用接口
public:
    PeriodMd() = default;

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_MARKDOWN_FORMATTER_H
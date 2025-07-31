// queries/formatters/IDailyReportFormatter.h
#ifndef I_DAILY_REPORT_FORMATTER_H
#define I_DAILY_REPORT_FORMATTER_H

#include <string>
#include <sqlite3.h>
#include "queries/_shared/DailyReportData.h" // Corrected: Was MonthlyReportData.h

/**
 * @class IDailyReportFormatter
 * @brief 日报格式化器的抽象接口（策略接口）。
 */
class IDailyReportFormatter {
public:
    virtual ~IDailyReportFormatter() = default;

    /**
     * @brief 格式化日报数据的纯虚函数。
     * @param data 从 Querier 获取的日报数据。
     * @param db 数据库连接指针。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const DailyReportData& data, sqlite3* db) const = 0; // Corrected: Was MonthlyReportData
};

#endif // I_DAILY_REPORT_FORMATTER_H
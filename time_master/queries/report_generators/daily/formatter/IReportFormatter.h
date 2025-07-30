// queries/report_generators/daily/formatter/IReportFormatter.h
#ifndef I_DAILY_REPORT_FORMATTER_H
#define I_DAILY_REPORT_FORMATTER_H

#include <string>
#include <sqlite3.h>
#include "queries/report_generators/daily/_daily_data/DailyReportData.h"

/**
 * @class IReportFormatter
 * @brief 日报格式化器的抽象接口（策略接口）。
 *
 * 定义了所有具体日报格式化器必须实现的通用接口。
 * 这允许在运行时动态地切换格式化策略。
 */
class IReportFormatter {
public:
    virtual ~IReportFormatter() = default;

    /**
     * @brief 格式化日报数据的纯虚函数。
     * @param data 从 Querier 获取的日报数据。
     * @param db 数据库连接指针。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const DailyReportData& data, sqlite3* db) = 0;
};

#endif // I_DAILY_REPORT_FORMATTER_H
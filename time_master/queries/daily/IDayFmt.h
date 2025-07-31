#ifndef I_DAILY_FMT_H
#define I_DAILY_FMT_H

#include <string>
#include <sqlite3.h>
#include "queries/shared/DailyReportData.h"

/**
 * @class IDayFmt
 * @brief 日报格式化器的抽象接口（策略接口）。
 */
class IDayFmt {
public:
    virtual ~IDayFmt() = default;

    /**
     * @brief 格式化日报数据的纯虚函数。
     * @param data 从 Querier 获取的日报数据。
     * @param db 数据库连接指针。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const DailyReportData& data, sqlite3* db) const = 0;
};

#endif // I_DAILY_FMT_H
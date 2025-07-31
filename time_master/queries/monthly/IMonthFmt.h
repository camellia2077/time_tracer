// queries/monthly/IMonthFmt.h
#ifndef I_MONTHLY_REPORT_FORMATTER_H
#define I_MONTHLY_REPORT_FORMATTER_H

#include <string>
#include <sqlite3.h>
#include "queries/shared/MonthlyReportData.h" // 注意：路径已更新

/**
 * @class IMonthFmt
 * @brief 月报格式化器的抽象接口（策略接口）。
 */
class IMonthFmt {
public:
    virtual ~IMonthFmt() = default;

    /**
     * @brief 格式化月报数据的纯虚函数。
     * @param data 从 Querier 获取的月报数据。
     * @param db 数据库连接指针。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const MonthlyReportData& data, sqlite3* db) const = 0;
};

#endif // I_MONTHLY_REPORT_FORMATTER_H

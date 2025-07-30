#ifndef I_PERIOD_REPORT_FORMATTER_H
#define I_PERIOD_REPORT_FORMATTER_H

#include <string>
#include <sqlite3.h>
#include "queries/report_generators/period/_period_data/PeriodReportData.h"

/**
 * @class IReportFormatter
 * @brief 周期报告格式化器的抽象接口（策略接口）。
 *
 * 定义了所有具体周期报告格式化器必须实现的通用方法，
 * 允许在运行时动态切换格式化策略。
 */
class IReportFormatter {
public:
    virtual ~IReportFormatter() = default;

    /**
     * @brief 格式化周期报告数据的纯虚函数。
     * @param data 从 Querier 获取的周期报告数据。
     * @param db 数据库连接指针。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const PeriodReportData& data, sqlite3* db) const = 0;
};

#endif // I_PERIOD_REPORT_FORMATTER_H
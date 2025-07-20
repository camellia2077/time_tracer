#ifndef I_MONTHLY_REPORT_FORMATTER_H
#define I_MONTHLY_REPORT_FORMATTER_H

#include <string>
#include <sqlite3.h>
#include "report_generators/_shared/query_data_structs.h"

/**
 * @class IReportFormatter
 * @brief 月报格式化器的抽象接口（策略接口）。
 *
 * 定义了所有具体月报格式化器必须实现的通用方法。
 * 这使得在运行时可以根据需要选择不同的格式化策略。
 */
class IReportFormatter {
public:
    virtual ~IReportFormatter() = default;

    /**
     * @brief 格式化月报数据的纯虚函数。
     * @param data 从 Querier 获取的月报数据。
     * @param db 数据库连接指针。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const MonthlyReportData& data, sqlite3* db) const = 0;
};

#endif // I_MONTHLY_REPORT_FORMATTER_H
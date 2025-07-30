// queries/report_generators/monthly/MonthlyReportGenerator.h
#ifndef MONTHLY_REPORT_GENERATOR_H
#define MONTHLY_REPORT_GENERATOR_H

#include <sqlite3.h>
#include <string>
#include "queries/report_generators/_shared/ReportFormat.h" // 引入报告格式的定义

/**
 * @class MonthlyReportGenerator
 * @brief 封装了获取和格式化月报的逻辑。
 * 这个类为生成完整的月报字符串提供了一个简单的接口。
 */
class MonthlyReportGenerator {
public:
    explicit MonthlyReportGenerator(sqlite3* db);

    /**
     * @brief 为指定月份生成格式化的月报。
     * @param year_month 报告的年月，格式为 YYYYMM。
     * @param format [修改] 需要生成的报告格式。
     * @return 包含格式化月报的字符串。
     */
    std::string generate_report(const std::string& year_month, ReportFormat format);

private:
    sqlite3* m_db;
};

#endif // MONTHLY_REPORT_GENERATOR_H
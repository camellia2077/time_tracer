#ifndef PERIOD_REPORT_GENERATOR_H
#define PERIOD_REPORT_GENERATOR_H

#include <sqlite3.h>
#include <string>
#include "queries/report_generators/_shared/ReportFormat.h" // [新增] 引入报告格式的定义

class PeriodReportGenerator {
public:
    explicit PeriodReportGenerator(sqlite3* db);

    /**
     * @brief 为指定的周期生成格式化的报告。
     * @param days 要查询的天数。
     * @param format [修改] 需要生成的报告格式。
     * @return 包含格式化周期报告的字符串。
     */
    std::string generate_report(int days, ReportFormat format);

private:
    sqlite3* m_db;
};

#endif // PERIOD_REPORT_GENERATOR_H
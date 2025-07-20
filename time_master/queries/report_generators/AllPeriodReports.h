#ifndef ALL_PERIOD_REPORTS_GENERATOR_H
#define ALL_PERIOD_REPORTS_GENERATOR_H

#include <sqlite3.h>
#include <vector>
#include "report_generators/_shared/query_data_structs.h"
#include "report_generators/_shared/ReportFormat.h" // [新增] 引入报告格式的定义

/**
 * @class AllPeriodReports
 * @brief 生成所有指定天数的周期报告。
 *
 * 该类接收一个包含多个天数的列表，为每个天数生成一个周期报告，
 * 并返回一个包含所有报告内容的 map。
 */
class AllPeriodReports {
public:
    /**
     * @brief 构造函数。
     * @param db 指向数据库连接的指针。
     */
    explicit AllPeriodReports(sqlite3* db);

    /**
     * @brief 根据提供的天数列表生成所有周期报告。
     * @param days_list 一个包含多个天数的 vector，例如 {7, 30, 90}。
     * @param format [修改] 需要生成的报告格式。
     * @return 一个 map，键是天数，值是格式化后的报告字符串。
     */
    FormattedPeriodReports generate_reports(const std::vector<int>& days_list, ReportFormat format);

private:
    sqlite3* m_db;
};

#endif // ALL_PERIOD_REPORTS_GENERATOR_H
#ifndef ALL_MONTHLY_REPORTS_GENERATOR_H
#define ALL_MONTHLY_REPORTS_GENERATOR_H

#include <sqlite3.h>
#include <map>
#include <string>
#include "report_generators/_shared/query_data_structs.h"


/**
 * @class AllMonthlyReports
 * @brief 生成所有月度报告，并按年月进行分类。
 *
 * 该类负责查询数据库中所有有记录的月份，调用相应的查询器和格式化器，
 * 并返回一个按年、月分类好的、包含所有报告内容的结构体。
 */
class AllMonthlyReports {
public:
    /**
     * @brief 构造函数。
     * @param db 指向数据库连接的指针。
     */
    explicit AllMonthlyReports(sqlite3* db);

    /**
     * @brief 生成所有月报并返回分类好的结果。
     * @return 一个包含所有格式化后月报的嵌套 map。
     */
    FormattedMonthlyReports generate_reports();

private:
    sqlite3* m_db;
};

#endif // ALL_MONTHLY_REPORTS_GENERATOR_H

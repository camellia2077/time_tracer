#ifndef ALL_REPORTS_GENERATOR_H
#define ALL_REPORTS_GENERATOR_H

#include <sqlite3.h>
#include "report_generators/_shared/query_data_structs.h"

/**
 * @class AllDayReports
 * @brief 生成所有日报，并按年月进行分类。
 *
 * 这个类负责查询数据库中的所有日报数据，
 * 复用 DailyReportFormatter 将其格式化，
 * 并返回一个按年、月分类好的数据结构。
 */
class AllDayReports {
public:
    /**
     * @brief 构造函数。
     * @param db 指向数据库连接的指针。
     */
    explicit AllDayReports(sqlite3* db);

    /**
     * @brief 生成所有日报并返回分类好的结果。
     * @return 一个包含所有格式化后日报的嵌套 map。
     */
    FormattedGroupedReports generate_all_reports();

private:
    sqlite3* m_db;
};

#endif // ALL_REPORTS_GENERATOR_H

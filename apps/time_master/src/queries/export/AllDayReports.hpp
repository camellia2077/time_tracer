// queries/export/AllDayReports.hpp
#ifndef ALL_REPORTS_GENERATOR_HPP
#define ALL_REPORTS_GENERATOR_HPP


#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp" // 引入报告格式的定义

#include <sqlite3.h>

/**
 * @class AllDayReports
 * @brief 生成所有日报，并按年月进行分类。
 *
 * 这个类负责查询数据库中的所有日报数据，
 * 并使用指定的格式化器将其格式化，
 * 然后返回一个按年、月分类好的数据结构。
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
     * @param format [修改] 需要生成的报告格式（例如 Markdown）。
     * @return 一个包含所有格式化后日报的嵌套 map。
     */
    FormattedGroupedReports generate_all_reports(ReportFormat format);

private:
    sqlite3* m_db;
};

#endif // ALL_REPORTS_GENERATOR_HPP
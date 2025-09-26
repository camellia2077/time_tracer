// queries/export/AllDayReports.hpp
#ifndef ALL_DAY_REPORTS_HPP
#define ALL_DAY_REPORTS_HPP 

#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/types/ReportFormat.hpp" 
#include "common/AppConfig.hpp" // [新增]

#include <sqlite3.h>
#include <string>

class AllDayReports {
public:
    /**
     * @brief 构造函数。
     * @param db 指向数据库连接的指针。
     * @param config 应用程序配置对象的引用。
     */
    explicit AllDayReports(sqlite3* db, const AppConfig& config);

    /**
     * @brief 生成所有日报并返回分类好的结果。
     * @param format 需要生成的报告格式。
     * @return 一个包含所有格式化后日报的嵌套 map。
     */
    FormattedGroupedReports generate_all_reports(ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_; // [修改]
};

#endif // ALL_DAY_REPORTS_HPP
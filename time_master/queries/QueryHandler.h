#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include <sqlite3.h>
#include <string>
#include "report_generators/_shared/query_data_structs.h" // [新增] 引入数据结构

/**
 * @class QueryHandler
 * @brief 所有查询功能的统一入口（外观类）。定义了我们统一的查询接口
 *
 * 该类为客户端（如 main 函数）提供一个简单的接口，
 * 内部则负责调用相应的 ReportGenerator 类来完成具体工作。
 */
class QueryHandler {
public:
    // 构造函数，接收数据库连接
    explicit QueryHandler(sqlite3* db);

    // 按天查询，返回报告字符串
    std::string run_daily_query(const std::string& date_str) const;

    // 按周期查询，返回报告字符串
    std::string run_period_query(int days) const;

    // 按月查询，返回报告字符串
    std::string run_monthly_query(const std::string& year_month_str) const;

    // --- [新增] 导出所有日报的功能 ---
    /**
     * @brief 获取所有日报，并按年月分类。
     * @return 一个包含所有格式化后日报的嵌套 map。
     */
    FormattedGroupedReports run_export_all_reports_query() const;

private:
    sqlite3* m_db; // 保存数据库连接
};

#endif // QUERY_HANDLER_H

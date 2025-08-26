// file: action_handler/query/DirectQueryManager.hpp

#ifndef DIRECT_QUERY_MANAGER_HPP
#define DIRECT_QUERY_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <memory> // 用于 std::unique_ptr
#include "queries/shared/ReportFormat.hpp" // 引入 ReportFormat

// 前向声明 (Forward Declaration)
class QueryHandler;

/**
 * @brief 直接查询管理器
 * @details
 * 该模块作为 ActionHandler 和 QueryHandler 之间的中间层,
 * 专门负责处理直接在控制台输出的查询请求。
 * 它封装了 QueryHandler 的创建和调用逻辑。
 */
class DirectQueryManager {
public:
    /**
     * @brief 构造函数
     * @param db 指向 sqlite3 数据库连接的指针
     */
    explicit DirectQueryManager(sqlite3* db);

    /**
     * @brief 析构函数
     */
    ~DirectQueryManager();

    // --- 查询执行方法 ---

    /**
     * @brief 执行单日查询
     * @param date 要查询的日期字符串 (格式: YYYY-MM-DD)
     * @param format 报告的输出格式 (如 Markdown, PlainText)
     * @return 返回格式化后的报告字符串
     */
    std::string run_daily_query(const std::string& date, ReportFormat format);

    /**
     * @brief 执行单月查询
     * @param month 要查询的月份字符串 (格式: YYYY-MM)
     * @param format 报告的输出格式
     * @return 返回格式化后的报告字符串
     */
    std::string run_monthly_query(const std::string& month, ReportFormat format);

    /**
     * @brief 执行周期查询
     * @param days 要查询的过去天数
     * @param format 报告的输出格式
     * @return 返回格式化后的报告字符串
     */
    std::string run_period_query(int days, ReportFormat format);

private:
    // 指向实际查询处理器的智能指针。
    // 使用 unique_ptr 来自动管理 QueryHandler 的生命周期。
    std::unique_ptr<QueryHandler> query_handler_;
};

#endif // DIRECT_QUERY_MANAGER_HPP
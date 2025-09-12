// file: action_handler/query/DirectQueryManager.cpp

 // 项目使用预编译头文件
#include "DirectQueryManager.hpp"
#include "queries/QueryHandler.hpp" // 引入底层的查询处理器

/**
 * @brief 构造函数实现
 * @details 初始化时, 创建一个 QueryHandler 实例来处理所有后续的查询请求。
 * @param db sqlite3 数据库连接指针
 */
DirectQueryManager::DirectQueryManager(sqlite3* db) {
    // 创建 QueryHandler 实例, 并将所有权交给智能指针
    query_handler_ = std::make_unique<QueryHandler>(db);
}

/**
 * @brief 析构函数实现
 * @details 默认析构函数即可, std::unique_ptr 会自动管理 QueryHandler 的生命周期。
 */
DirectQueryManager::~DirectQueryManager() = default;

/**
 * @brief 执行单日查询
 * @details 将请求直接委托给底层的 QueryHandler 来执行。
 * @param date 日期字符串
 * @param format 报告格式
 * @return 报告字符串
 */
std::string DirectQueryManager::run_daily_query(const std::string& date, ReportFormat format) {
    // 调用 QueryHandler 的同名方法
    return query_handler_->run_daily_query(date, format);
}

/**
 * @brief 执行单月查询
 * @details 将请求直接委托给底层的 QueryHandler 来执行。
 * @param month 月份字符串
 * @param format 报告格式
 * @return 报告字符串
 */
std::string DirectQueryManager::run_monthly_query(const std::string& month, ReportFormat format) {
    // 调用 QueryHandler 的同名方法
    return query_handler_->run_monthly_query(month, format);
}

/**
 * @brief 执行周期查询
 * @details 将请求直接委托给底层的 QueryHandler 来执行。
 * @param days 天数
 * @param format 报告格式
 * @return 报告字符串
 */
std::string DirectQueryManager::run_period_query(int days, ReportFormat format) {
    // 调用 QueryHandler 的同名方法
    return query_handler_->run_period_query(days, format);
}
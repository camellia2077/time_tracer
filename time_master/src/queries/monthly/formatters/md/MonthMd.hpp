// queries/monthly/formatters/MonthMd.hpp
#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"  
#include "queries/shared/data/MonthlyReportData.hpp" // 为模板类型引入定义
#include <sstream>

// Forward declaration
struct MonthlyReportData;

/**
 * @class MonthMd
 * @brief 将月报数据格式化为 Markdown 字符串的具体实现。
 */
class MonthMd : public IReportFormatter<MonthlyReportData> { // 继承自模板化通用接口
public:
    MonthMd() = default;

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP
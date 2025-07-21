// MonthlyTex.h (已更新)
#ifndef MONTHLY_REPORT_TEX_FORMATTER_H
#define MONTHLY_REPORT_TEX_FORMATTER_H

#include "report_generators/monthly/formatter/IReportFormatter.h"
#include "report_generators/_shared/BaseTexFormatter.h" // <-- 1. 包含新的基类头文件

// --- 前置声明 ---
struct DailyReportData;
struct PeriodReportData;

/**
 * @class MonthlyTex
 * @brief 将月报数据格式化为 TeX 字符串的具体实现。
 * 通过继承 BaseTexFormatter 来复用文档框架。
 */
// 2. 同时继承接口和实现基类
class MonthlyTex : public IReportFormatter, private BaseTexFormatter {
public:
    MonthlyTex() = default;

    // 3. 公开的 format_report 接口将调用基类的模板方法
    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    // 4. 实现基类定义的纯虚“钩子”函数，用于填充核心内容
    void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const override;

    // -- 为满足基类的纯虚函数要求，提供空的实现 --
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const override {}

    // 5. _display_preamble 方法已被移除
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_H
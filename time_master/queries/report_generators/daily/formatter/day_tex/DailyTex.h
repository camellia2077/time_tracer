// DailyTex.h (已更新)
#ifndef DAILY_REPORT_TEX_FORMATTER_H
#define DAILY_REPORT_TEX_FORMATTER_H

#include "queries/report_generators/daily/formatter/IReportFormatter.h"
#include "queries/report_generators/_shared/BaseTexFormatter.h" // <-- 1. 包含新的基类头文件

// --- 前置声明 ---
struct MonthlyReportData;
struct PeriodReportData;

/**
 * @class DailyTex
 * @brief 将日报数据格式化为 TeX 字符串的具体实现。
 * 通过继承 BaseTexFormatter 来复用文档框架。
 */
 // 2. 同时继承接口和实现基类
class DailyTex : public IReportFormatter, private BaseTexFormatter {
public:
    DailyTex() = default;

    // 3. 公开的 format_report 接口将调用基类的模板方法
    std::string format_report(const DailyReportData& data, sqlite3* db) override;

private:
    // 4. 实现基类定义的纯虚“钩子”函数，用于填充核心内容
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override;
    
    // -- 为满足基类的纯虚函数要求，提供空的实现 --
    void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const override {}

    // 5. _display_preamble 方法已被移除，因为它的功能已被基类取代
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_TEX_FORMATTER_H
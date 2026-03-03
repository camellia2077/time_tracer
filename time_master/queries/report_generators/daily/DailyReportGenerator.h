// queries/report_generators/daily/DailyReportGenerator.h
#ifndef DAILY_REPORT_GENERATOR_H
#define DAILY_REPORT_GENERATOR_H

#include <sqlite3.h>
#include <string>
#include "queries/report_generators/_shared/ReportFormat.h" // [新增] 引入报告格式的定义

/**
 * @class DailyReportGenerator
 * @brief 封装了获取和格式化日报的逻辑。
 * 这个类为生成完整的日报字符串提供了一个简单的接口。
 */
class DailyReportGenerator {
public:
    /**
     * @brief DailyReportGenerator 的构造函数。
     * @param db 指向 SQLite 数据库连接的指针。
     */
    explicit DailyReportGenerator(sqlite3* db);

    /**
     * @brief 为指定日期生成格式化的日报。
     * @param date 报告的日期，格式为 YYYYMMDD。
     * @param format [修改] 需要生成的报告格式（例如 Markdown）。
     * @return 包含格式化日报的字符串。
     */
    std::string generate_report(const std::string& date, ReportFormat format);

private:
    sqlite3* m_db;
};

#endif // DAILY_REPORT_GENERATOR_H
// queries/daily/DayGenerator.hpp
#ifndef DAILY_REPORT_GENERATOR_HPP
#define DAILY_REPORT_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/types/ReportFormat.hpp" // 报告格式的定义
#include "common/AppConfig.hpp"

/**
 * @class DayGenerator
 * @brief 封装了获取和格式化日报的逻辑。
 * 这个类为生成完整的日报字符串提供了一个简单的接口。
 */
class DayGenerator {
public:
    /**
     * @brief DayGenerator 的构造函数。
     * @param db 指向 SQLite 数据库连接的指针。
     * @param config 应用程序配置对象的引用。
     */
    explicit DayGenerator(sqlite3* db, const AppConfig& config);

    /**
     * @brief 为指定日期生成格式化的日报。
     * @param date 报告的日期，格式为 YYYYMMDD。
     * @param format 需要生成的报告格式（例如 Markdown）。
     * @return 包含格式化日报的字符串。
     */
    std::string generate_report(const std::string& date, ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_; // [修改] 现在只存储 AppConfig 的引用
};

#endif // DAILY_REPORT_GENERATOR_HPP
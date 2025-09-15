// queries/monthly/MonthGenerator.hpp
#ifndef MONTHLY_REPORT_GENERATOR_HPP
#define MONTHLY_REPORT_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/ReportFormat.hpp" // 引入报告格式的定义

/**
 * @class MonthGenerator
 * @brief 封装了获取和格式化月报的逻辑。
 * 这个类为生成完整的月报字符串提供了一个简单的接口。
 */
class MonthGenerator {
public:
    explicit MonthGenerator(sqlite3* db, const std::string& month_typ_config_path);

    /**
     * @brief 为指定月份生成格式化的月报。
     * @param year_month 报告的年月，格式为 YYYYMM。
     * @param format [修改] 需要生成的报告格式。
     * @return 包含格式化月报的字符串。
     */
    std::string generate_report(const std::string& year_month, ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_month_typ_config_path;
};

#endif // MONTHLY_REPORT_GENERATOR_HPP
// queries/shared/Interface/IReportFormatter.hpp
#ifndef I_REPORT_FORMATTER_HPP
#define I_REPORT_FORMATTER_HPP

#include <string>
#include <sqlite3.h>

/**
 * @class IReportFormatter
 * @brief 通用的报告格式化器模板接口（策略接口）。
 * @tparam ReportDataType 报告所依赖的数据结构类型
 * (例如 DailyReportData, MonthlyReportData, PeriodReportData)。
 */
template<typename ReportDataType>
class IReportFormatter {
public:
    virtual ~IReportFormatter() = default;

    /**
     * @brief 格式化报告数据的纯虚函数。
     * @param data 从 Querier 获取的报告数据。
     * @return 格式化后的报告字符串。
     */
    virtual std::string format_report(const ReportDataType& data) const = 0;
};

#endif // I_REPORT_FORMATTER_HPP
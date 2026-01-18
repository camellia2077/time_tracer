// reports/shared/interfaces/i_report_formatter.hpp
#ifndef REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_HPP_
#define REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_HPP_

#include <string>
#include <sqlite3.h>
#include "reports/shared/model/daily_report_data.hpp"
#include "reports/shared/model/monthly_report_data.hpp"
#include "reports/shared/model/period_report_data.hpp"

template<typename ReportDataType>
class IReportFormatter {
public:
    virtual ~IReportFormatter() = default;
    virtual std::string format_report(const ReportDataType& data) const = 0;
};

// C-style interface for DLLs
#ifdef __cplusplus
extern "C" {
#endif

typedef void* FormatterHandle;

// [核心修改] 创建函数现在接收配置的 JSON 字符串内容，而不是 AppConfig 对象
// 外部模块负责读取文件，将内容传给这里
typedef FormatterHandle (*CreateFormatterFunc)(const char* config_json);

typedef void (*DestroyFormatterFunc)(FormatterHandle);

// 为不同数据类型定义不同的 format 函数指针
typedef const char* (*FormatReportFunc_Day)(FormatterHandle, const DailyReportData&);
typedef const char* (*FormatReportFunc_Month)(FormatterHandle, const MonthlyReportData&);
typedef const char* (*FormatReportFunc_Period)(FormatterHandle, const PeriodReportData&);

#ifdef __cplusplus
}
#endif

#endif // REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_HPP_
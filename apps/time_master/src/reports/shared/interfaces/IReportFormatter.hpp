// reports/shared/interfaces/IReportFormatter.hpp
#ifndef I_REPORT_FORMATTER_HPP
#define I_REPORT_FORMATTER_HPP

#include <string>
#include <sqlite3.h>
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/shared/data/MonthlyReportData.hpp" // [新增] 引入月报数据结构
#include "common/AppConfig.hpp"

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
typedef FormatterHandle (*CreateFormatterFunc)(const AppConfig&);
typedef void (*DestroyFormatterFunc)(FormatterHandle);

// [核心修改] 为不同数据类型定义不同的 format 函数指针
typedef const char* (*FormatReportFunc_Day)(FormatterHandle, const DailyReportData&);
typedef const char* (*FormatReportFunc_Month)(FormatterHandle, const MonthlyReportData&);

#ifdef __cplusplus
}
#endif

#endif // I_REPORT_FORMATTER_HPP
// infrastructure/reports/shared/interfaces/i_report_formatter.hpp
#ifndef REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_H_
#define REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_H_

#include <sqlite3.h>

#include <string>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/monthly_report_data.hpp"
#include "domain/reports/models/period_report_data.hpp"
#include "domain/reports/models/range_report_data.hpp"
#include "domain/reports/models/weekly_report_data.hpp"
#include "domain/reports/models/yearly_report_data.hpp"

template <typename ReportDataType>
class IReportFormatter {
 public:
  virtual ~IReportFormatter() = default;
  [[nodiscard]] virtual auto FormatReport(const ReportDataType& data) const
      -> std::string = 0;
};

// C-style interface for DLLs
#ifdef __cplusplus
extern "C" {
#endif

using FormatterHandle = void*;

// [核心修改] 创建函数现在接收配置的 JSON 字符串内容，而不是 AppConfig 对象
// 外部模块负责读取文件，将内容传给这里
using CreateFormatterFunc = FormatterHandle (*)(const char* config_json);

using DestroyFormatterFunc = void (*)(FormatterHandle);

// 为不同数据类型定义不同的 format 函数指针
using FormatReportFunc_Day = const char* (*)(FormatterHandle,
                                             const DailyReportData&);
using FormatReportFunc_Month = const char* (*)(FormatterHandle,
                                               const MonthlyReportData&);
using FormatReportFunc_Period = const char* (*)(FormatterHandle,
                                                const PeriodReportData&);
using FormatReportFunc_Range = const char* (*)(FormatterHandle,
                                               const RangeReportData&);

#ifdef __cplusplus
}
#endif

#endif  // REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_H_

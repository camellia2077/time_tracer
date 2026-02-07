// infrastructure/reports/formatter_registry.cpp
#include "infrastructure/reports/formatter_registry.hpp"

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/monthly_report_data.hpp"
#include "domain/reports/models/period_report_data.hpp"
#include "domain/reports/models/weekly_report_data.hpp"
#include "domain/reports/models/yearly_report_data.hpp"
#include "domain/reports/types/report_format.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

namespace reports {
void RegisterReportFormatters() {
  static const bool kIsRegistered = []() -> bool {
    // --- 注册日报格式 (DailyReportData) ---
    using DayFactory = GenericFormatterFactory<DailyReportData>;
    DayFactory::RegisterDllFormatter(ReportFormat::kMarkdown, "DayMdFormatter");
    DayFactory::RegisterDllFormatter(ReportFormat::kTyp, "DayTypFormatter");
    DayFactory::RegisterDllFormatter(ReportFormat::kLaTeX, "DayTexFormatter");

    // --- 注册月报格式 (MonthlyReportData) ---
    using MonthFactory = GenericFormatterFactory<MonthlyReportData>;
    MonthFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                       "MonthMdFormatter");
    MonthFactory::RegisterDllFormatter(ReportFormat::kTyp, "MonthTypFormatter");
    MonthFactory::RegisterDllFormatter(ReportFormat::kLaTeX,
                                       "MonthTexFormatter");

    // --- 注册周期报告格式 (PeriodReportData) ---
    using PeriodFactory = GenericFormatterFactory<PeriodReportData>;
    PeriodFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                        "RangeMdFormatter");
    PeriodFactory::RegisterDllFormatter(ReportFormat::kLaTeX,
                                        "RangeTexFormatter");
    PeriodFactory::RegisterDllFormatter(ReportFormat::kTyp,
                                        "RangeTypFormatter");

    // --- 注册周报格式 (WeeklyReportData) ---
    using WeekFactory = GenericFormatterFactory<WeeklyReportData>;
    WeekFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                      "RangeMdFormatter");
    WeekFactory::RegisterDllFormatter(ReportFormat::kLaTeX,
                                      "RangeTexFormatter");
    WeekFactory::RegisterDllFormatter(ReportFormat::kTyp, "RangeTypFormatter");

    // --- 注册年报格式 (YearlyReportData) ---
    using YearFactory = GenericFormatterFactory<YearlyReportData>;
    YearFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                      "RangeMdFormatter");
    YearFactory::RegisterDllFormatter(ReportFormat::kLaTeX,
                                      "RangeTexFormatter");
    YearFactory::RegisterDllFormatter(ReportFormat::kTyp, "RangeTypFormatter");

    return true;
  }();

  (void)kIsRegistered;
}
}  // namespace reports

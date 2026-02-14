// domain/reports/models/period_report_models.hpp
#ifndef DOMAIN_REPORTS_MODELS_PERIOD_REPORT_MODELS_H_
#define DOMAIN_REPORTS_MODELS_PERIOD_REPORT_MODELS_H_

#include "domain/reports/models/range_report_data.hpp"

struct PeriodReportData : public RangeReportData {};
struct WeeklyReportData : public RangeReportData {};
struct MonthlyReportData : public RangeReportData {};
struct YearlyReportData : public RangeReportData {};

#endif  // DOMAIN_REPORTS_MODELS_PERIOD_REPORT_MODELS_H_

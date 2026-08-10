// domain/insights/models/period_insights_models.hpp
#ifndef DOMAIN_INSIGHTS_MODELS_PERIOD_INSIGHTS_MODELS_H_
#define DOMAIN_INSIGHTS_MODELS_PERIOD_INSIGHTS_MODELS_H_

#include "domain/insights/models/range_insights_data.hpp"

struct PeriodInsightsData : public RangeInsightsData {};
struct WeeklyInsightsData : public RangeInsightsData {};
struct MonthlyInsightsData : public RangeInsightsData {};
struct YearlyInsightsData : public RangeInsightsData {};

#endif  // DOMAIN_INSIGHTS_MODELS_PERIOD_INSIGHTS_MODELS_H_

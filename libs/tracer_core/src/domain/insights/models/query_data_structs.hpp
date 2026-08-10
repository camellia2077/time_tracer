// domain/insights/models/query_data_structs.hpp
#ifndef DOMAIN_INSIGHTS_MODELS_QUERY_DATA_STRUCTS_H_
#define DOMAIN_INSIGHTS_MODELS_QUERY_DATA_STRUCTS_H_

#include <map>
#include <string>
#include <vector>

struct FormattedDailyInsightsEntry {
  std::string insights_id;
  std::string kContent;
};

using FormattedDailyInsightsByMonth =
    std::map<int, std::vector<FormattedDailyInsightsEntry>>;
using FormattedGroupedInsights = std::map<int, FormattedDailyInsightsByMonth>;

using FormattedInsightsByMonth = std::map<int, std::string>;
using FormattedMonthlyInsights = std::map<int, FormattedInsightsByMonth>;

using FormattedPeriodInsights = std::map<int, std::string>;

using FormattedInsightsByIsoWeek = std::map<int, std::string>;
using FormattedWeeklyInsights = std::map<int, FormattedInsightsByIsoWeek>;

using FormattedYearlyInsights = std::map<int, std::string>;

#endif  // DOMAIN_INSIGHTS_MODELS_QUERY_DATA_STRUCTS_H_

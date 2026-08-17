// infra/insights/data/queriers/utils/batch_aggregation.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_UTILS_BATCH_AGGREGATION_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_UTILS_BATCH_AGGREGATION_H_

#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "domain/insights/interfaces/i_project_info_provider.hpp"
#include "infra/insights/data/utils/project_tree_builder.hpp"

namespace insights::data::batch {

template <typename InsightsDataT>
void FinalizeAggregation(
    InsightsDataT& data,
    const std::map<std::int64_t, std::int64_t>& project_agg, int actual_days,
    const IProjectInfoProvider& provider) {
  data.actual_days = actual_days;
  data.project_stats.clear();
  data.project_stats.reserve(project_agg.size());
  for (const auto& [project_id, duration] : project_agg) {
    data.project_stats.emplace_back(project_id, duration);
  }
  if (data.activity.total_duration_seconds > 0) {
    data.project_tree.clear();
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider);
  }
}

template <typename InsightsDataT>
void FinalizeAggregation(
    InsightsDataT& data,
    const std::map<std::int64_t, std::int64_t>& project_agg,
    const std::set<std::string>& distinct_dates,
    const IProjectInfoProvider& provider) {
  FinalizeAggregation(data, project_agg,
                      static_cast<int>(distinct_dates.size()), provider);
}

template <typename InsightsDataT>
void FinalizeAggregationFromStats(InsightsDataT& data, int actual_days,
                                  const IProjectInfoProvider& provider) {
  data.actual_days = actual_days;
  if (data.activity.total_duration_seconds > 0) {
    data.project_tree.clear();
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider);
  }
}

template <typename KeyT, typename InsightsDataT>
void FinalizeGroupedAggregation(
    std::map<KeyT, InsightsDataT>& results,
    const std::map<KeyT, std::map<std::int64_t, std::int64_t>>& project_agg,
    const std::map<KeyT, std::set<std::string>>& distinct_dates,
    const IProjectInfoProvider& provider) {
  for (auto& [key, data] : results) {
    auto agg_it = project_agg.find(key);
    auto date_it = distinct_dates.find(key);
    if (agg_it == project_agg.end() || date_it == distinct_dates.end()) {
      data.actual_days = 0;
      continue;
    }
    FinalizeAggregation(data, agg_it->second,
                        static_cast<int>(date_it->second.size()), provider);
  }
}

template <typename KeyT, typename InsightsDataT>
void FinalizeGroupedAggregationWithDays(
    std::map<KeyT, InsightsDataT>& results,
    const std::map<KeyT, std::map<std::int64_t, std::int64_t>>& project_agg,
    const std::map<KeyT, int>& actual_days,
    const IProjectInfoProvider& provider) {
  for (auto& [key, data] : results) {
    auto agg_it = project_agg.find(key);
    auto days_it = actual_days.find(key);
    int days = (days_it != actual_days.end()) ? days_it->second : 0;

    if (agg_it == project_agg.end()) {
      data.actual_days = days;
      data.project_stats.clear();
      data.project_tree.clear();
      continue;
    }

    FinalizeAggregation(data, agg_it->second, days, provider);
  }
}

}  // namespace insights::data::batch

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_UTILS_BATCH_AGGREGATION_H_

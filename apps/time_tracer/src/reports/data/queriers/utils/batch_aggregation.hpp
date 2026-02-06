// reports/data/queriers/utils/batch_aggregation.hpp
#ifndef REPORTS_DATA_QUERIERS_UTILS_BATCH_AGGREGATION_H_
#define REPORTS_DATA_QUERIERS_UTILS_BATCH_AGGREGATION_H_

#include <map>
#include <set>
#include <string>

#include "reports/data/interfaces/i_project_info_provider.hpp"
#include "reports/data/utils/project_tree_builder.hpp"

namespace reports::data::batch {

template <typename ReportDataT>
void FinalizeAggregation(ReportDataT& data,
                         const std::map<long long, long long>& project_agg,
                         int actual_days,
                         const IProjectInfoProvider& provider) {
  data.actual_days = actual_days;
  data.project_stats.clear();
  data.project_stats.reserve(project_agg.size());
  for (const auto& [project_id, duration] : project_agg) {
    data.project_stats.emplace_back(project_id, duration);
  }
  if (data.total_duration > 0) {
    data.project_tree.clear();
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider);
  }
}

template <typename ReportDataT>
void FinalizeAggregation(ReportDataT& data,
                         const std::map<long long, long long>& project_agg,
                         const std::set<std::string>& distinct_dates,
                         const IProjectInfoProvider& provider) {
  FinalizeAggregation(data, project_agg,
                      static_cast<int>(distinct_dates.size()), provider);
}

template <typename ReportDataT>
void FinalizeAggregationFromStats(ReportDataT& data, int actual_days,
                                  const IProjectInfoProvider& provider) {
  data.actual_days = actual_days;
  if (data.total_duration > 0) {
    data.project_tree.clear();
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider);
  }
}

template <typename KeyT, typename ReportDataT>
void FinalizeGroupedAggregation(
    std::map<KeyT, ReportDataT>& results,
    const std::map<KeyT, std::map<long long, long long>>& project_agg,
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

template <typename KeyT, typename ReportDataT>
void FinalizeGroupedAggregationWithDays(
    std::map<KeyT, ReportDataT>& results,
    const std::map<KeyT, std::map<long long, long long>>& project_agg,
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

}  // namespace reports::data::batch

#endif  // REPORTS_DATA_QUERIERS_UTILS_BATCH_AGGREGATION_H_

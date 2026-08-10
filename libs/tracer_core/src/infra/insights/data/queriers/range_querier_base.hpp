// infra/insights/data/queriers/range_querier_base.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

#include "infra/insights/data/cache/project_name_cache.hpp"
#include "infra/insights/data/queriers/base_querier.hpp"
#include "infra/insights/data/utils/project_tree_builder.hpp"

template <typename InsightsDataType, typename QueryParamType>
class RangeQuerierBase : public BaseQuerier<InsightsDataType, QueryParamType> {
 public:
  explicit RangeQuerierBase(sqlite3* sqlite_db, QueryParamType query_param)
      : BaseQuerier<InsightsDataType, QueryParamType>(sqlite_db, query_param) {}

  [[nodiscard]] auto FetchData() -> InsightsDataType override {
    InsightsDataType data =
        BaseQuerier<InsightsDataType, QueryParamType>::FetchData();

    this->FetchActualDays(data);
    data.matched_day_count = this->FetchMatchedDayRowCount();
    data.matched_record_count = this->FetchMatchedRecordCount();
    data.has_records = data.matched_record_count > 0;
    const auto kFlagCounts = this->FetchDayFlagCounts();
    data.status_true_days = kFlagCounts.status_true_days;
    data.exercise_true_days = kFlagCounts.exercise_true_days;
    data.cardio_true_days = kFlagCounts.cardio_true_days;
    data.anaerobic_true_days = kFlagCounts.anaerobic_true_days;

    if (data.total_duration > 0) {
      ProjectNameCache name_cache;
      name_cache.EnsureLoaded(this->db_);
      BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                              name_cache);
    }

    return data;
  }
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

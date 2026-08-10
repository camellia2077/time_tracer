// infra/insights/data/queriers/range_querier_base.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

#include "infra/insights/data/cache/project_name_cache.hpp"
#include "infra/insights/data/queriers/base_querier.hpp"
#include "infra/insights/data/utils/daily_status_utils.hpp"
#include "infra/insights/data/utils/project_tree_builder.hpp"

template <typename InsightsDataType, typename QueryParamType>
class RangeQuerierBase : public BaseQuerier<InsightsDataType, QueryParamType> {
 public:
  explicit RangeQuerierBase(sqlite3* sqlite_db, QueryParamType query_param,
                            const DailyStatusConfig* status_config = nullptr)
      : BaseQuerier<InsightsDataType, QueryParamType>(sqlite_db, query_param),
        status_config_(status_config) {}

  [[nodiscard]] auto FetchData() -> InsightsDataType override {
    InsightsDataType data =
        BaseQuerier<InsightsDataType, QueryParamType>::FetchData();

    this->FetchActualDays(data);
    data.matched_day_count = this->FetchMatchedDayRowCount();
    data.matched_record_count = this->FetchMatchedRecordCount();
    data.has_records = data.matched_record_count > 0;
    if (data.total_duration > 0 ||
        (status_config_ != nullptr && !status_config_->statuses.empty())) {
      ProjectNameCache name_cache;
      name_cache.EnsureLoaded(this->db_);
      if (!data.project_stats.empty()) {
        BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                                name_cache);
      }
      if (status_config_ != nullptr) {
        data.statuses = BuildStatusValues<InsightsStatusValue>(
            data.project_tree, *status_config_);
      }
    }

    return data;
  }

  const DailyStatusConfig* status_config_ = nullptr;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

// infrastructure/reports/data/queriers/range_querier_base.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/queriers/base_querier.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"

template <typename ReportDataType, typename QueryParamType>
class RangeQuerierBase : public BaseQuerier<ReportDataType, QueryParamType> {
 public:
  explicit RangeQuerierBase(sqlite3* sqlite_db, QueryParamType query_param)
      : BaseQuerier<ReportDataType, QueryParamType>(sqlite_db, query_param) {}

  [[nodiscard]] auto FetchData() -> ReportDataType override {
    ReportDataType data =
        BaseQuerier<ReportDataType, QueryParamType>::FetchData();

    this->FetchActualDays(data);
    const auto kFlagCounts = this->FetchDayFlagCounts();
    data.status_true_days = kFlagCounts.status_true_days;
    data.sleep_true_days = kFlagCounts.sleep_true_days;
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

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

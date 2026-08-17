// infra/insights/data/queriers/range_querier_base.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

#include "infra/insights/data/cache/project_name_cache.hpp"
#include "infra/insights/data/queriers/base_querier.hpp"
#include "infra/insights/data/utils/activity_record_mapper.hpp"
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
    data.activity.occurrence_count = this->FetchMatchedRecordCount();
    data.has_records = data.activity.occurrence_count > 0;
    if (data.has_records ||
        (status_config_ != nullptr && !status_config_->statuses.empty())) {
      ProjectNameCache name_cache;
      name_cache.EnsureLoaded(this->db_);
      if (data.has_records) {
        FetchActivityDays(data, name_cache);
      }
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

 private:
  void FetchActivityDays(InsightsDataType& data,
                         const IProjectInfoProvider& provider) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT ";
    sql += schema::time_records::db::kDate;
    sql += ", ";
    sql += schema::time_records::db::kStart;
    sql += ", ";
    sql += schema::time_records::db::kEnd;
    sql += ", ";
    sql += schema::time_records::db::kProjectId;
    sql += ", ";
    sql += schema::time_records::db::kDuration;
    sql += ", ";
    sql += schema::time_records::db::kActivityRemark;
    sql += ", ";
    sql += schema::time_records::db::kLogicalId;
    sql += ", ";
    sql += schema::time_records::db::kRecordKind;
    sql += " FROM ";
    sql += schema::time_records::db::kTable;
    sql += " WHERE ";
    sql += this->GetDateConditionSql();
    sql += " ORDER BY ";
    sql += schema::time_records::db::kDate;
    sql += " DESC, ";
    sql += schema::time_records::db::kLogicalId;
    sql += " ASC;";

    if (sqlite3_prepare_v2(this->db_, sql.c_str(), -1, &stmt, nullptr) !=
        SQLITE_OK) {
      sqlite3_finalize(stmt);
      return;
    }
    this->BindSqlParameters(stmt);
    DailyInsightsData* current_day = nullptr;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const auto* date_value = sqlite3_column_text(stmt, 0);
      if (date_value == nullptr) {
        continue;
      }
      const std::string date(reinterpret_cast<const char*>(date_value));
      if (current_day == nullptr || current_day->date != date) {
        data.activity_days.push_back({.date = date});
        current_day = &data.activity_days.back();
      }

      TimeRecord record = tracer::core::infrastructure::insights::data::
          record_mapping::ReadTimeRecord(stmt,
                                         {.start_time = 1,
                                          .end_time = 2,
                                          .project_id = 3,
                                          .duration = 4,
                                          .activity_remark = 5,
                                          .logical_id = 6,
                                          .record_kind = 7},
                                         provider);
      current_day->activity.Add(record.duration_seconds);
      current_day->detailed_records.push_back(std::move(record));
    }
    sqlite3_finalize(stmt);
  }
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_QUERIER_BASE_H_

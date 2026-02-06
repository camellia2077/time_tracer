// reports/data/queriers/monthly/month_querier.cpp
#include "reports/data/queriers/monthly/month_querier.hpp"

#include <algorithm>
#include <cctype>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/utils/project_tree_builder.hpp"

MonthQuerier::MonthQuerier(sqlite3* sqlite_db, std::string_view year_month)
    : BaseQuerier(sqlite_db, year_month) {}

auto MonthQuerier::FetchData() -> MonthlyReportData {
  MonthlyReportData data = BaseQuerier::FetchData();

  FetchActualDays(data);

  if (data.total_duration > 0) {
    // [新增] 获取并确保缓存加载
    auto& name_cache = ProjectNameCache::Instance();
    name_cache.EnsureLoaded(db_);

    // [核心修改] 传入 name_cache 替代 db_
    BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                                name_cache);
  }

  return data;
}

auto MonthQuerier::ValidateInput() const -> bool {
  if (this->param_.length() != static_cast<size_t>(kYearMonthLength)) {
    return false;
  }
  if (this->param_[kDashPosition] != '-') {
    return false;
  }

  return (std::isdigit(this->param_[0]) != 0) &&
         (std::isdigit(this->param_[1]) != 0) &&
         (std::isdigit(this->param_[2]) != 0) &&
         (std::isdigit(this->param_[3]) != 0) &&
         (std::isdigit(this->param_[kMonthStartPosition]) != 0) &&
         (std::isdigit(this->param_[kMonthEndPosition]) != 0);
}

void MonthQuerier::HandleInvalidInput(MonthlyReportData& data) const {
  data.is_valid = false;
}

void MonthQuerier::PrepareData(MonthlyReportData& data) const {
  data.range_label = std::string(param_);
  data.start_date = std::string(param_) + "-01";
  data.end_date = std::string(param_) + "-31";
  data.requested_days = 0;
}

auto MonthQuerier::GetDateConditionSql() const -> std::string {
  return "date >= ? AND date <= ?";
}

void MonthQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  std::string start_date = std::string(param_) + "-01";
  std::string end_date = std::string(param_) + "-31";

  sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
}

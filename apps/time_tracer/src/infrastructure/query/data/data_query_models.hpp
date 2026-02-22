#pragma once

#include <optional>
#include <string>

namespace time_tracer::infrastructure::query::data {

struct QueryFilters {
  std::optional<int> kYear;
  std::optional<int> kMonth;
  std::optional<std::string> from_date;
  std::optional<std::string> to_date;
  std::optional<std::string> remark;
  std::optional<std::string> day_remark;
  std::optional<std::string> project;
  std::optional<std::string> root;
  std::optional<int> exercise;
  std::optional<int> status;
  bool overnight = false;
  bool reverse = false;
  std::optional<int> limit;
};

}  // namespace time_tracer::infrastructure::query::data

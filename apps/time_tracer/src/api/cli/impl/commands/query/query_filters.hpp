// api/cli/impl/commands/query/query_filters.hpp
#pragma once

#include <optional>
#include <string>

namespace time_tracer::cli::impl::commands::query {

struct QueryFilters {
  std::optional<int> year;
  std::optional<int> month;
  std::optional<std::string> from_date;
  std::optional<std::string> to_date;
  std::optional<std::string> remark;
  std::optional<std::string> day_remark;
  std::optional<std::string> project;
  std::optional<int> exercise;
  std::optional<int> status;
  bool overnight = false;
  bool reverse = false;
  std::optional<int> limit;
};

}  // namespace time_tracer::cli::impl::commands::query

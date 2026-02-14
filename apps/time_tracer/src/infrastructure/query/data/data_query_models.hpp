// infrastructure/query/data/data_query_models.hpp
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"

namespace time_tracer::infrastructure::query::data {

struct QueryFilters {
  std::optional<int> kYear;
  std::optional<int> kMonth;
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

[[nodiscard]] auto RenderList(const std::vector<std::string>& items)
    -> std::string;

auto PrintList(const std::vector<std::string>& items) -> void;

[[nodiscard]] auto RenderDayDurations(const std::vector<DayDurationRow>& rows)
    -> std::string;

auto PrintDayDurations(const std::vector<DayDurationRow>& rows) -> void;

[[nodiscard]] auto RenderDayDurationStats(const DayDurationStats& stats)
    -> std::string;

auto PrintDayDurationStats(const DayDurationStats& stats) -> void;

[[nodiscard]] auto RenderTopDayDurations(
    const std::vector<DayDurationRow>& rows, int top_n) -> std::string;

auto PrintTopDayDurations(const std::vector<DayDurationRow>& rows, int top_n)
    -> void;

[[nodiscard]] auto ComputeDayDurationStats(
    const std::vector<DayDurationRow>& rows) -> DayDurationStats;

}  // namespace time_tracer::infrastructure::query::data

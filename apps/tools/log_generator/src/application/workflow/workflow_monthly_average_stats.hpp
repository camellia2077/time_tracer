#ifndef APPLICATION_WORKFLOW_WORKFLOW_MONTHLY_AVERAGE_STATS_HPP_
#define APPLICATION_WORKFLOW_WORKFLOW_MONTHLY_AVERAGE_STATS_HPP_

#include <string>
#include <unordered_set>
#include <vector>

namespace App::workflow_stats {

struct MonthlyAverageStat {
  int year = 0;
  int month = 0;
  int day_count = 0;
  double average_minutes_per_day = 0.0;
};

auto BuildMonthlyAverageStat(
    int year, int month, const std::string& month_content,
    const std::unordered_set<std::string>& wake_keywords) -> MonthlyAverageStat;

void PrintMonthlyAverageReport(std::vector<MonthlyAverageStat> stats);

}  // namespace App::workflow_stats

#endif  // APPLICATION_WORKFLOW_WORKFLOW_MONTHLY_AVERAGE_STATS_HPP_

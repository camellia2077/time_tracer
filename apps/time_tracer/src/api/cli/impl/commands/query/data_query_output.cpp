// api/cli/impl/commands/query/data_query_output.cpp
#include "api/cli/impl/commands/query/data_query_output.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace time_tracer::cli::impl::commands::query::data {
namespace {

constexpr int kDefaultOutputPrecision = 6;

[[nodiscard]] auto FormatDurationSeconds(double seconds) -> std::string {
  if (seconds <= 0.0) {
    return TimeFormatDuration(0);
  }
  return TimeFormatDuration(std::llround(seconds));
}

}  // namespace

auto PrintList(const std::vector<std::string>& items) -> void {
  for (const auto& item : items) {
    std::cout << item << "\n";
  }
  std::cout << "Total: " << items.size() << std::endl;
}

auto PrintDayDurations(const std::vector<DayDurationRow>& rows) -> void {
  for (const auto& row : rows) {
    std::cout << row.date << " " << TimeFormatDuration(row.total_seconds)
              << "\n";
  }
  std::cout << "Total: " << rows.size() << std::endl;
}

auto PrintDayDurationStats(const DayDurationStats& stats) -> void {
  constexpr double kSecondsPerHour = 3600.0;
  std::cout << "Days: " << stats.count << "\n";
  if (stats.count == 0) {
    std::cout << "Average: 0h 0m\n";
    std::cout << "Median: 0h 0m\n";
    std::cout << "P25: 0h 0m\n";
    std::cout << "P75: 0h 0m\n";
    std::cout << "P90: 0h 0m\n";
    std::cout << "P95: 0h 0m\n";
    std::cout << "Min: 0h 0m\n";
    std::cout << "Max: 0h 0m\n";
    std::cout << "IQR: 0h 0m\n";
    std::cout << "MAD: 0h 0m\n";
    std::cout << "Variance (h^2): 0.00\n";
    std::cout << "Std Dev: 0.00h (0h 0m)\n";
    std::cout << "\nNotes:\n";
    std::cout << "- Median: middle value after sorting daily durations.\n";
    std::cout << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
    std::cout << "- IQR: P75 - P25, robust spread measure.\n";
    std::cout << "- MAD: median(|x - median|), robust dispersion.\n";
    return;
  }

  std::cout << "Average: " << FormatDurationSeconds(stats.mean_seconds) << "\n";
  std::cout << "Median: " << FormatDurationSeconds(stats.median_seconds)
            << "\n";
  std::cout << "P25: " << FormatDurationSeconds(stats.p25_seconds) << "\n";
  std::cout << "P75: " << FormatDurationSeconds(stats.p75_seconds) << "\n";
  std::cout << "P90: " << FormatDurationSeconds(stats.p90_seconds) << "\n";
  std::cout << "P95: " << FormatDurationSeconds(stats.p95_seconds) << "\n";
  std::cout << "Min: " << FormatDurationSeconds(stats.min_seconds) << "\n";
  std::cout << "Max: " << FormatDurationSeconds(stats.max_seconds) << "\n";
  std::cout << "IQR: " << FormatDurationSeconds(stats.iqr_seconds) << "\n";
  std::cout << "MAD: " << FormatDurationSeconds(stats.mad_seconds) << "\n";

  const double kVarianceHours =
      stats.variance_seconds / (kSecondsPerHour * kSecondsPerHour);
  const double kStdHours = stats.stddev_seconds / kSecondsPerHour;

  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Variance (h^2): " << kVarianceHours << "\n";
  std::cout << "Std Dev: " << kStdHours << "h ("
            << FormatDurationSeconds(stats.stddev_seconds) << ")\n";

  std::cout << "\nNotes:\n";
  std::cout << "- Median: middle value after sorting daily durations.\n";
  std::cout << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
  std::cout << "- IQR: P75 - P25, robust spread measure.\n";
  std::cout << "- MAD: median(|x - median|), robust dispersion.\n";

  std::cout.unsetf(std::ios::floatfield);
  std::cout << std::setprecision(kDefaultOutputPrecision);
}

auto PrintTopDayDurations(const std::vector<DayDurationRow>& rows, int top_n)
    -> void {
  if (top_n <= 0 || rows.empty()) {
    return;
  }
  const int kCount = std::min(static_cast<int>(rows.size()), top_n);

  std::cout << "\nTop " << kCount << " longest days:\n";
  for (int index = 0; index < kCount; ++index) {
    const auto& row = rows[rows.size() - 1 - index];
    std::cout << "  " << row.date << " "
              << TimeFormatDuration(row.total_seconds) << "\n";
  }

  std::cout << "Top " << kCount << " shortest days:\n";
  for (int index = 0; index < kCount; ++index) {
    const auto& row = rows[index];
    std::cout << "  " << row.date << " "
              << TimeFormatDuration(row.total_seconds) << "\n";
  }
}

}  // namespace time_tracer::cli::impl::commands::query::data

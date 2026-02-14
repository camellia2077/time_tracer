// infrastructure/query/data/data_query_output.cpp
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace time_tracer::infrastructure::query::data {
namespace {

constexpr int kDefaultOutputPrecision = 6;

[[nodiscard]] auto FormatDurationSeconds(double seconds) -> std::string {
  if (seconds <= 0.0) {
    return TimeFormatDuration(0);
  }
  return TimeFormatDuration(std::llround(seconds));
}

auto AppendDayDurationStats(std::ostream& output_stream,
                            const DayDurationStats& stats) -> void {
  constexpr double kSecondsPerHour = 3600.0;
  output_stream << "Days: " << stats.count << "\n";
  if (stats.count == 0) {
    output_stream << "Average: 0h 0m\n";
    output_stream << "Median: 0h 0m\n";
    output_stream << "P25: 0h 0m\n";
    output_stream << "P75: 0h 0m\n";
    output_stream << "P90: 0h 0m\n";
    output_stream << "P95: 0h 0m\n";
    output_stream << "Min: 0h 0m\n";
    output_stream << "Max: 0h 0m\n";
    output_stream << "IQR: 0h 0m\n";
    output_stream << "MAD: 0h 0m\n";
    output_stream << "Variance (h^2): 0.00\n";
    output_stream << "Std Dev: 0.00h (0h 0m)\n";
    output_stream << "\nNotes:\n";
    output_stream << "- Median: middle value after sorting daily durations.\n";
    output_stream << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
    output_stream << "- IQR: P75 - P25, robust spread measure.\n";
    output_stream << "- MAD: median(|x - median|), robust dispersion.\n";
    return;
  }

  output_stream << "Average: " << FormatDurationSeconds(stats.mean_seconds)
                << "\n";
  output_stream << "Median: " << FormatDurationSeconds(stats.median_seconds)
                << "\n";
  output_stream << "P25: " << FormatDurationSeconds(stats.p25_seconds) << "\n";
  output_stream << "P75: " << FormatDurationSeconds(stats.p75_seconds) << "\n";
  output_stream << "P90: " << FormatDurationSeconds(stats.p90_seconds) << "\n";
  output_stream << "P95: " << FormatDurationSeconds(stats.p95_seconds) << "\n";
  output_stream << "Min: " << FormatDurationSeconds(stats.min_seconds) << "\n";
  output_stream << "Max: " << FormatDurationSeconds(stats.max_seconds) << "\n";
  output_stream << "IQR: " << FormatDurationSeconds(stats.iqr_seconds) << "\n";
  output_stream << "MAD: " << FormatDurationSeconds(stats.mad_seconds) << "\n";

  const double kVarianceHours =
      stats.variance_seconds / (kSecondsPerHour * kSecondsPerHour);
  const double kStdHours = stats.stddev_seconds / kSecondsPerHour;

  output_stream << std::fixed << std::setprecision(2);
  output_stream << "Variance (h^2): " << kVarianceHours << "\n";
  output_stream << "Std Dev: " << kStdHours << "h ("
                << FormatDurationSeconds(stats.stddev_seconds) << ")\n";

  output_stream << "\nNotes:\n";
  output_stream << "- Median: middle value after sorting daily durations.\n";
  output_stream << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
  output_stream << "- IQR: P75 - P25, robust spread measure.\n";
  output_stream << "- MAD: median(|x - median|), robust dispersion.\n";

  output_stream.unsetf(std::ios::floatfield);
  output_stream << std::setprecision(kDefaultOutputPrecision);
}

}  // namespace

auto RenderList(const std::vector<std::string>& items) -> std::string {
  std::ostringstream output_stream;
  for (const auto& item : items) {
    output_stream << item << "\n";
  }
  output_stream << "Total: " << items.size() << "\n";
  return output_stream.str();
}

auto PrintList(const std::vector<std::string>& items) -> void {
  time_tracer::domain::ports::EmitInfo(RenderList(items));
}

auto RenderDayDurations(const std::vector<DayDurationRow>& rows)
    -> std::string {
  std::ostringstream output_stream;
  for (const auto& row : rows) {
    output_stream << row.date << " " << TimeFormatDuration(row.total_seconds)
                  << "\n";
  }
  output_stream << "Total: " << rows.size() << "\n";
  return output_stream.str();
}

auto PrintDayDurations(const std::vector<DayDurationRow>& rows) -> void {
  time_tracer::domain::ports::EmitInfo(RenderDayDurations(rows));
}

auto RenderDayDurationStats(const DayDurationStats& stats) -> std::string {
  std::ostringstream output_stream;
  AppendDayDurationStats(output_stream, stats);
  return output_stream.str();
}

auto PrintDayDurationStats(const DayDurationStats& stats) -> void {
  time_tracer::domain::ports::EmitInfo(RenderDayDurationStats(stats));
}

auto RenderTopDayDurations(const std::vector<DayDurationRow>& rows, int top_n)
    -> std::string {
  if (top_n <= 0 || rows.empty()) {
    return "";
  }
  const int kCount = std::min(static_cast<int>(rows.size()), top_n);

  std::ostringstream output_stream;
  output_stream << "\nTop " << kCount << " longest days:\n";
  for (int index = 0; index < kCount; ++index) {
    const auto& row = rows[rows.size() - 1 - index];
    output_stream << "  " << row.date << " "
                  << TimeFormatDuration(row.total_seconds) << "\n";
  }

  output_stream << "Top " << kCount << " shortest days:\n";
  for (int index = 0; index < kCount; ++index) {
    const auto& row = rows[index];
    output_stream << "  " << row.date << " "
                  << TimeFormatDuration(row.total_seconds) << "\n";
  }

  return output_stream.str();
}

auto PrintTopDayDurations(const std::vector<DayDurationRow>& rows, int top_n)
    -> void {
  time_tracer::domain::ports::EmitInfo(RenderTopDayDurations(rows, top_n));
}

}  // namespace time_tracer::infrastructure::query::data

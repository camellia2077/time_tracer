#include "infrastructure/query/data/renderers/text_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace time_tracer::infrastructure::query::data::renderers {
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
  output_stream << "## Day Duration Stats\n\n";
  output_stream << "- **Days**: " << stats.count << "\n";
  if (stats.count == 0) {
    output_stream << "- **Average**: 0h 0m\n";
    output_stream << "- **Median**: 0h 0m\n";
    output_stream << "- **P25**: 0h 0m\n";
    output_stream << "- **P75**: 0h 0m\n";
    output_stream << "- **P90**: 0h 0m\n";
    output_stream << "- **P95**: 0h 0m\n";
    output_stream << "- **Min**: 0h 0m\n";
    output_stream << "- **Max**: 0h 0m\n";
    output_stream << "- **IQR**: 0h 0m\n";
    output_stream << "- **MAD**: 0h 0m\n";
    output_stream << "- **Variance (h^2)**: 0.00\n";
    output_stream << "- **Std Dev**: 0.00h (0h 0m)\n";
    output_stream << "\n### Notes\n";
    output_stream << "- Median: middle value after sorting daily durations.\n";
    output_stream << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
    output_stream << "- IQR: P75 - P25, robust spread measure.\n";
    output_stream << "- MAD: median(|x - median|), robust dispersion.\n";
    return;
  }

  output_stream << "- **Average**: "
                << FormatDurationSeconds(stats.mean_seconds) << "\n";
  output_stream << "- **Median**: "
                << FormatDurationSeconds(stats.median_seconds) << "\n";
  output_stream << "- **P25**: " << FormatDurationSeconds(stats.p25_seconds)
                << "\n";
  output_stream << "- **P75**: " << FormatDurationSeconds(stats.p75_seconds)
                << "\n";
  output_stream << "- **P90**: " << FormatDurationSeconds(stats.p90_seconds)
                << "\n";
  output_stream << "- **P95**: " << FormatDurationSeconds(stats.p95_seconds)
                << "\n";
  output_stream << "- **Min**: " << FormatDurationSeconds(stats.min_seconds)
                << "\n";
  output_stream << "- **Max**: " << FormatDurationSeconds(stats.max_seconds)
                << "\n";
  output_stream << "- **IQR**: " << FormatDurationSeconds(stats.iqr_seconds)
                << "\n";
  output_stream << "- **MAD**: " << FormatDurationSeconds(stats.mad_seconds)
                << "\n";

  const double kVarianceHours =
      stats.variance_seconds / (kSecondsPerHour * kSecondsPerHour);
  const double kStdHours = stats.stddev_seconds / kSecondsPerHour;

  output_stream << std::fixed << std::setprecision(2);
  output_stream << "- **Variance (h^2)**: " << kVarianceHours << "\n";
  output_stream << "- **Std Dev**: " << kStdHours << "h ("
                << FormatDurationSeconds(stats.stddev_seconds) << ")\n";

  output_stream << "\n### Notes\n";
  output_stream << "- Median: middle value after sorting daily durations.\n";
  output_stream << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
  output_stream << "- IQR: P75 - P25, robust spread measure.\n";
  output_stream << "- MAD: median(|x - median|), robust dispersion.\n";

  output_stream.unsetf(std::ios::floatfield);
  output_stream << std::setprecision(kDefaultOutputPrecision);
}

struct NamedNodeView {
  std::string_view name;
  const reporting::ProjectNode* node = nullptr;
};

[[nodiscard]] auto CollectSortedChildren(const reporting::ProjectNode& node)
    -> std::vector<NamedNodeView> {
  std::vector<NamedNodeView> children;
  children.reserve(node.children.size());
  for (const auto& [name, child] : node.children) {
    children.push_back({.name = name, .node = &child});
  }
  std::ranges::sort(
      children, [](const NamedNodeView& lhs, const NamedNodeView& rhs) -> bool {
        return lhs.name < rhs.name;
      });
  return children;
}

void AppendTreeChildren(std::ostringstream& output_stream,
                        const reporting::ProjectNode& node,
                        const std::string& prefix, int current_depth,
                        int max_depth) {
  if (max_depth >= 0 && current_depth >= max_depth) {
    return;
  }

  const auto kChildren = CollectSortedChildren(node);
  for (size_t index = 0; index < kChildren.size(); ++index) {
    const auto& child = kChildren[index];
    const bool kIsLast = (index + 1 == kChildren.size());
    output_stream << prefix << (kIsLast ? "└── " : "├── ") << child.name
                  << "\n";
    const std::string kNextPrefix = prefix + (kIsLast ? "    " : "│   ");
    AppendTreeChildren(output_stream, *child.node, kNextPrefix,
                       current_depth + 1, max_depth);
  }
}

}  // namespace

auto RenderListText(const std::vector<std::string>& items) -> std::string {
  std::ostringstream output_stream;
  for (const auto& item : items) {
    output_stream << item << "\n";
  }
  output_stream << "Total: " << items.size() << "\n";
  return output_stream.str();
}

auto RenderDayDurationsText(const std::vector<DayDurationRow>& rows)
    -> std::string {
  std::ostringstream output_stream;
  for (const auto& row : rows) {
    output_stream << row.date << " " << TimeFormatDuration(row.total_seconds)
                  << "\n";
  }
  output_stream << "Total: " << rows.size() << "\n";
  return output_stream.str();
}

auto RenderDayDurationStatsText(const DayDurationStats& stats) -> std::string {
  std::ostringstream output_stream;
  AppendDayDurationStats(output_stream, stats);
  return output_stream.str();
}

auto RenderTopDayDurationsText(const std::vector<DayDurationRow>& rows,
                               int top_n) -> std::string {
  if (top_n <= 0 || rows.empty()) {
    return "";
  }
  const int kCount = std::min(static_cast<int>(rows.size()), top_n);

  std::ostringstream output_stream;
  output_stream << "\n### Top " << kCount << " Longest Days\n";
  for (int index = 0; index < kCount; ++index) {
    const auto& row = rows[rows.size() - 1 - index];
    output_stream << "- " << row.date << " "
                  << TimeFormatDuration(row.total_seconds) << "\n";
  }

  output_stream << "\n### Top " << kCount << " Shortest Days\n";
  for (int index = 0; index < kCount; ++index) {
    const auto& row = rows[index];
    output_stream << "- " << row.date << " "
                  << TimeFormatDuration(row.total_seconds) << "\n";
  }

  return output_stream.str();
}

auto RenderDayDurationStatsOutputText(const std::vector<DayDurationRow>& rows,
                                      const DayDurationStats& stats,
                                      const std::optional<int>& top_n)
    -> std::string {
  std::string content = RenderDayDurationStatsText(stats);
  if (top_n.has_value()) {
    content += RenderTopDayDurationsText(rows, *top_n);
  }
  return content;
}

auto RenderActivitySuggestionsText(
    const std::vector<ActivitySuggestionRow>& rows) -> std::string {
  std::ostringstream output_stream;
  output_stream << std::fixed << std::setprecision(2);
  for (const auto& row : rows) {
    const std::string kLastUsed =
        row.last_used_date.empty() ? "-" : row.last_used_date;
    output_stream << row.activity_name << " | score: " << row.score
                  << " | count: " << row.usage_count << " | duration: "
                  << TimeFormatDuration(row.total_duration_seconds)
                  << " | last: " << kLastUsed << "\n";
  }
  output_stream.unsetf(std::ios::floatfield);
  output_stream << std::setprecision(kDefaultOutputPrecision);
  output_stream << "Total: " << rows.size() << "\n";
  return output_stream.str();
}

auto RenderProjectTreeText(const reporting::ProjectTree& tree, int max_depth)
    -> std::string {
  std::ostringstream output_stream;
  if (tree.empty()) {
    output_stream << "Total: 0\n";
    return output_stream.str();
  }

  std::vector<NamedNodeView> roots;
  roots.reserve(tree.size());
  for (const auto& [name, node] : tree) {
    roots.push_back({.name = name, .node = &node});
  }
  std::ranges::sort(
      roots, [](const NamedNodeView& lhs, const NamedNodeView& rhs) -> bool {
        return lhs.name < rhs.name;
      });

  for (const auto& root : roots) {
    output_stream << root.name << "\n";
    AppendTreeChildren(output_stream, *root.node, "", 0, max_depth);
  }
  return output_stream.str();
}

auto RenderJsonObjectText(std::string content) -> std::string {
  return content;
}

}  // namespace time_tracer::infrastructure::query::data::renderers

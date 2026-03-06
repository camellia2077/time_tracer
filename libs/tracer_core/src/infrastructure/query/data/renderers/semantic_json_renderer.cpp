// infrastructure/query/data/renderers/semantic_json_renderer.cpp
#include "infrastructure/query/data/renderers/semantic_json_renderer.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tracer_core::infrastructure::query::data::renderers {
namespace {

constexpr int kSemanticSchemaVersion = 1;

using nlohmann::json;

[[nodiscard]] auto BuildSemanticBase(std::string_view action) -> json {
  return json{
      {"schema_version", kSemanticSchemaVersion},
      {"action", action},
      {"output_mode", "semantic_json"},
  };
}

[[nodiscard]] auto BuildDayDurationRowJson(const DayDurationRow& row) -> json {
  return json{
      {"date", row.date},
      {"duration_seconds", row.total_seconds},
  };
}

[[nodiscard]] auto BuildDayDurationStatsJson(const DayDurationStats& stats)
    -> json {
  return json{
      {"count", stats.count},
      {"mean_seconds", stats.mean_seconds},
      {"variance_seconds", stats.variance_seconds},
      {"stddev_seconds", stats.stddev_seconds},
      {"median_seconds", stats.median_seconds},
      {"p25_seconds", stats.p25_seconds},
      {"p75_seconds", stats.p75_seconds},
      {"p90_seconds", stats.p90_seconds},
      {"p95_seconds", stats.p95_seconds},
      {"min_seconds", stats.min_seconds},
      {"max_seconds", stats.max_seconds},
      {"iqr_seconds", stats.iqr_seconds},
      {"mad_seconds", stats.mad_seconds},
  };
}

[[nodiscard]] auto BuildTopRows(const std::vector<DayDurationRow>& rows,
                                int top_n, bool longest) -> json {
  json out = json::array();
  if (top_n <= 0 || rows.empty()) {
    return out;
  }

  const int kCount = std::min(static_cast<int>(rows.size()), top_n);
  for (int index = 0; index < kCount; ++index) {
    const size_t kRowIndex = longest
                                 ? rows.size() - 1 - static_cast<size_t>(index)
                                 : static_cast<size_t>(index);
    out.push_back(BuildDayDurationRowJson(rows[kRowIndex]));
  }
  return out;
}

struct NamedNodeRef {
  std::string_view name;
  const ProjectTreeNode* node = nullptr;
};

[[nodiscard]] auto CollectSortedChildren(const ProjectTreeNode& node)
    -> std::vector<NamedNodeRef> {
  std::vector<NamedNodeRef> children;
  children.reserve(node.children.size());
  for (const auto& child : node.children) {
    children.push_back({.name = child.name, .node = &child});
  }
  std::ranges::sort(
      children, [](const NamedNodeRef& lhs, const NamedNodeRef& rhs) -> bool {
        return lhs.name < rhs.name;
      });
  return children;
}

[[nodiscard]] auto ResolveNodeDurationSeconds(const ProjectTreeNode& node)
    -> long long {
  return node.duration_seconds.value_or(0LL);
}

void AppendProjectNodeChildren(json& out_children, const ProjectTreeNode& node,
                               int current_depth, int max_depth) {
  if (max_depth >= 0 && current_depth >= max_depth) {
    return;
  }

  const auto kChildren = CollectSortedChildren(node);
  for (const auto& child : kChildren) {
    json child_json = {
        {"name", child.name},
        {"duration_seconds", ResolveNodeDurationSeconds(*child.node)},
        {"children", json::array()},
    };
    AppendProjectNodeChildren(child_json["children"], *child.node,
                              current_depth + 1, max_depth);
    out_children.push_back(std::move(child_json));
  }
}

}  // namespace

auto BuildSemanticListPayload(std::string_view action,
                              const std::vector<std::string>& items)
    -> std::string {
  json payload = BuildSemanticBase(action);
  payload["items"] = items;
  payload["total_count"] = items.size();
  return payload.dump();
}

auto BuildSemanticDayDurationsPayload(std::string_view action,
                                      const std::vector<DayDurationRow>& rows)
    -> std::string {
  json payload = BuildSemanticBase(action);
  payload["rows"] = json::array();
  for (const auto& row : rows) {
    payload["rows"].push_back(BuildDayDurationRowJson(row));
  }
  payload["total_count"] = rows.size();
  return payload.dump();
}

auto BuildSemanticDayStatsPayload(const std::vector<DayDurationRow>& rows,
                                  const DayDurationStats& stats,
                                  const std::optional<int>& top_n)
    -> std::string {
  json payload = BuildSemanticBase("days_stats");
  payload["stats"] = BuildDayDurationStatsJson(stats);
  payload["rows"] = json::array();
  for (const auto& row : rows) {
    payload["rows"].push_back(BuildDayDurationRowJson(row));
  }
  payload["total_count"] = rows.size();
  if (top_n.has_value() && *top_n > 0) {
    payload["top_n_requested"] = *top_n;
    payload["top_longest_rows"] = BuildTopRows(rows, *top_n, true);
    payload["top_shortest_rows"] = BuildTopRows(rows, *top_n, false);
  }
  return payload.dump();
}

auto BuildSemanticActivitySuggestionsPayload(
    const std::vector<ActivitySuggestionRow>& rows) -> std::string {
  json payload = BuildSemanticBase("activity_suggest");
  payload["items"] = json::array();
  for (const auto& row : rows) {
    payload["items"].push_back(json{
        {"activity_name", row.activity_name},
        {"usage_count", row.usage_count},
        {"total_duration_seconds", row.total_duration_seconds},
        {"last_used_date", row.last_used_date},
        {"score", row.score},
    });
  }
  payload["total_count"] = rows.size();
  return payload.dump();
}

auto BuildSemanticTreePayload(const std::vector<ProjectTreeNode>& nodes,
                              int max_depth) -> std::string {
  json payload = BuildSemanticBase("tree");
  payload["max_depth"] = max_depth;
  payload["roots"] = json::array();

  std::vector<NamedNodeRef> roots;
  roots.reserve(nodes.size());
  for (const auto& node : nodes) {
    roots.push_back({.name = node.name, .node = &node});
  }
  std::ranges::sort(
      roots, [](const NamedNodeRef& lhs, const NamedNodeRef& rhs) -> bool {
        return lhs.name < rhs.name;
      });

  for (const auto& root : roots) {
    json root_json = {
        {"name", root.name},
        {"duration_seconds", ResolveNodeDurationSeconds(*root.node)},
        {"children", json::array()},
    };
    AppendProjectNodeChildren(root_json["children"], *root.node, 0, max_depth);
    payload["roots"].push_back(std::move(root_json));
  }
  payload["root_count"] = roots.size();
  return payload.dump();
}

auto BuildSemanticJsonObjectPayload(std::string_view action,
                                    std::string content) -> std::string {
  json payload;
  try {
    payload = json::parse(content);
  } catch (...) {
    json wrapped = BuildSemanticBase(action);
    wrapped["raw_content"] = std::move(content);
    return wrapped.dump();
  }

  if (!payload.is_object()) {
    json wrapped = BuildSemanticBase(action);
    wrapped["data"] = std::move(payload);
    return wrapped.dump();
  }

  payload["schema_version"] = kSemanticSchemaVersion;
  payload["action"] = action;
  payload["output_mode"] = "semantic_json";
  return payload.dump();
}

}  // namespace tracer_core::infrastructure::query::data::renderers

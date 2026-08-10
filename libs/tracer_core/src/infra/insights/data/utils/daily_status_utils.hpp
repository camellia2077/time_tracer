// infra/insights/data/utils/daily_status_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_UTILS_DAILY_STATUS_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_UTILS_DAILY_STATUS_UTILS_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "domain/insights/models/daily_insights_data.hpp"
#include "domain/insights/models/project_tree.hpp"
#include "infra/config/models/insights_config_models.hpp"

inline auto SplitDailyStatusParent(std::string_view parent)
    -> std::vector<std::string_view> {
  std::vector<std::string_view> parts;
  std::size_t start = 0;
  while (start <= parent.size()) {
    const std::size_t separator = parent.find('/', start);
    const std::size_t end =
        separator == std::string_view::npos ? parent.size() : separator;
    if (end == start) {
      return {};
    }
    parts.emplace_back(parent.substr(start, end - start));
    if (separator == std::string_view::npos) {
      break;
    }
    start = separator + 1;
  }
  return parts;
}

inline auto FindDailyStatusProjectNode(const insights::ProjectTree& tree,
                                       std::string_view parent)
    -> const insights::ProjectNode* {
  const std::vector<std::string_view> parent_parts =
      SplitDailyStatusParent(parent);
  if (parent_parts.empty()) {
    return nullptr;
  }

  const auto root = tree.find(std::string(parent_parts.front()));
  if (root == tree.end()) {
    return nullptr;
  }
  const insights::ProjectNode* node = &root->second;
  for (std::size_t index = 1; index < parent_parts.size(); ++index) {
    const auto child = node->children.find(std::string(parent_parts[index]));
    if (child == node->children.end()) {
      return nullptr;
    }
    node = &child->second;
  }
  return node;
}

template <typename StatusValueT>
inline auto BuildStatusValues(
    const insights::ProjectTree& project_tree, const DailyStatusConfig& config)
    -> std::vector<StatusValueT> {
  std::vector<StatusValueT> values;
  values.reserve(config.statuses.size());
  for (const auto& status : config.statuses) {
    StatusValueT value{.id = status.id, .label = status.label};
    if (const auto* node = FindDailyStatusProjectNode(project_tree, status.parent);
        node != nullptr) {
      value.occurrence_count = static_cast<int>(node->occurrence_count);
      value.total_duration = node->duration;
    }
    values.push_back(std::move(value));
  }
  return values;
}

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_UTILS_DAILY_STATUS_UTILS_H_

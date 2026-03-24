#pragma once

#include <string>
#include <utility>
#include <vector>

#include "domain/reports/models/project_tree.hpp"
#include "shared/utils/string_utils.hpp"

namespace tracer::core::infrastructure::query::data::internal {

inline auto BuildProjectTreeFromRecords(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records) -> void {
  using tracer::core::shared::string_utils::SplitString;

  for (const auto& [project_path, duration] : records) {
    const std::vector<std::string> kParts = SplitString(project_path, '_');
    if (kParts.empty()) {
      continue;
    }

    tree[kParts.front()].duration += duration;
    reporting::ProjectNode* current_node = &tree[kParts.front()];

    for (size_t index = 1; index < kParts.size(); ++index) {
      current_node->children[kParts[index]].duration += duration;
      current_node = &current_node->children[kParts[index]];
    }
  }
}

}  // namespace tracer::core::infrastructure::query::data::internal

namespace tracer_core::infrastructure::query::data::internal {

using tracer::core::infrastructure::query::data::internal::
    BuildProjectTreeFromRecords;

}  // namespace tracer_core::infrastructure::query::data::internal

#ifndef TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TREE_HPP_
#define TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TREE_HPP_

#include <optional>
#include <string>
#include <vector>

namespace tracer::core::application::config {

// Presentation-neutral node kind for the activity hierarchy.
enum class ActivityHierarchyNodeKind {
  kLeaf,
  kGroup,
};

// Stable Core-owned representation of one canonical TOML node. `path` is the
// canonical path relative to the document's [canonical] table. The vector
// order is the source TOML order; consumers may sort for presentation.
struct ActivityHierarchyTreeNode {
  std::string canonical_key;
  std::string path;
  ActivityHierarchyNodeKind kind = ActivityHierarchyNodeKind::kLeaf;
  std::vector<std::string> aliases;
  std::vector<ActivityHierarchyTreeNode> children;

  [[nodiscard]] auto IsGroup() const noexcept -> bool {
    return kind == ActivityHierarchyNodeKind::kGroup;
  }
};

// Stable Core-owned representation of one canonical TOML document. `parent` is
// the document-level canonical prefix and is intentionally not a selectable
// node; selectable nodes are in `nodes` and their descendants. `color` is an
// optional presentation hint for this parent.
struct ActivityHierarchyTree {
  std::string parent;
  std::optional<std::string> color;
  std::vector<ActivityHierarchyTreeNode> nodes;
};

// Source-compatible names retained for existing Core callers.
using ActivityHierarchyNodeSnapshot = ActivityHierarchyTreeNode;
using ActivityHierarchySnapshot = ActivityHierarchyTree;

}  // namespace tracer::core::application::config

#endif  // TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TREE_HPP_

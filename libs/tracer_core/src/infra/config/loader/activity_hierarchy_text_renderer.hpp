#ifndef INFRASTRUCTURE_CONFIG_LOADER_ACTIVITY_HIERARCHY_TEXT_RENDERER_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_ACTIVITY_HIERARCHY_TEXT_RENDERER_HPP_

#include <string>

#include "application/ports/config/activity_hierarchy_tree.hpp"

namespace tracer::core::infrastructure::config::loader::detail {

[[nodiscard]] auto RenderActivityHierarchyTreeText(
    const application::config::ActivityHierarchyTree& tree, bool show_aliases)
    -> std::string;

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_ACTIVITY_HIERARCHY_TEXT_RENDERER_HPP_

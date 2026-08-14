#include "application/ports/config/activity_hierarchy_toml_editor.hpp"

#include "infra/config/loader/activity_hierarchy_document.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace config = tracer::core::application::config;

namespace {

using Replacement = config::AliasCanonicalReplacement;

struct Match {
  std::string canonical;
  std::vector<std::string> path;
  bool is_group = false;
};

auto CollectMatches(
    const tracer::core::infrastructure::config::loader::detail::ActivityHierarchyDocument&
        document,
    std::map<std::string, Match>& matches) -> void {
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectActivityHierarchyCanonicalNodes(document)) {
    matches.emplace(
        node.canonical,
        Match{node.canonical, node.path,
              node.kind == tracer::core::infrastructure::config::loader::
                               detail::ActivityHierarchyDocumentNodeKind::kGroup});
  }
}

[[nodiscard]] auto SplitPath(std::string_view path)
    -> std::vector<std::string> {
  if (path.empty()) {
    throw std::invalid_argument("Canonical target path must not be empty.");
  }
  std::vector<std::string> parts;
  std::size_t start = 0;
  while (start <= path.size()) {
    const auto kEnd = path.find('.', start);
    const auto kPart =
        path.substr(start, kEnd == std::string_view::npos ? path.size() - start
                                                         : kEnd - start);
    if (kPart.empty()) {
      throw std::invalid_argument(
          "Canonical target path contains an empty component.");
    }
    parts.emplace_back(kPart);
    if (kEnd == std::string_view::npos) {
      break;
    }
    start = kEnd + 1;
  }
  return parts;
}

[[nodiscard]] auto FindTable(toml::table& canonical,
                             const std::vector<std::string>& groups)
    -> toml::table* {
  toml::table* current = &canonical;
  for (const auto& group : groups) {
    auto* child = current->get(group);
    if (child == nullptr || !child->is_table()) {
      return nullptr;
    }
    current = child->as_table();
  }
  return current;
}

auto ValidateNewKey(std::string_view new_name) -> void {
  if (new_name.empty() || new_name == "group_aliases" ||
      new_name.find('.') != std::string_view::npos || new_name.find('_') == 0) {
    throw std::invalid_argument("New canonical key is invalid: " +
                                std::string(new_name));
  }
}

auto RenameOne(
    const Replacement& replacement, toml::table& canonical,
    const tracer::core::infrastructure::config::loader::detail::ActivityHierarchyDocument&
        document) -> void {
  std::map<std::string, Match> matches;
  CollectMatches(document, matches);
  const auto kSource = matches.find(replacement.old_canonical);
  if (kSource == matches.end()) {
    throw std::invalid_argument("Canonical not found: " +
                                replacement.old_canonical);
  }
  if (matches.contains(replacement.new_canonical)) {
    throw std::invalid_argument("Canonical already exists: " +
                                replacement.new_canonical);
  }

  const auto kSeparator = replacement.new_canonical.rfind('_');
  const std::string kPrefix =
      kSeparator == std::string::npos
          ? std::string{}
          : replacement.new_canonical.substr(0, kSeparator);
  const std::string kExpectedPrefix =
      kSource->second.canonical.substr(0, kSource->second.canonical.rfind('_'));
  if (kPrefix != kExpectedPrefix) {
    throw std::invalid_argument(
        "Canonical rename must keep the node at the same hierarchy level: " +
        replacement.old_canonical + " -> " + replacement.new_canonical);
  }
  const std::string kNewKey =
      kSeparator == std::string::npos
          ? replacement.new_canonical
          : replacement.new_canonical.substr(kSeparator + 1);
  if (kNewKey.empty() || kNewKey == "group_aliases") {
    throw std::invalid_argument("New canonical key is invalid: " + kNewKey);
  }

  std::vector<std::string> parent_path = kSource->second.path;
  parent_path.pop_back();
  toml::table* source_parent = FindTable(canonical, parent_path);
  if (source_parent == nullptr) {
    throw std::invalid_argument("Canonical not found: " +
                                replacement.old_canonical);
  }

  const std::string& source_key = kSource->second.path.back();
  toml::table group_copy;
  toml::array aliases_copy;
  toml::node& source_node = source_parent->at(source_key);
  const bool kIsGroup = source_node.is_table();
  if (kIsGroup) {
    group_copy = *source_node.as_table();
  } else if (source_node.is_array()) {
    aliases_copy = *source_node.as_array();
  } else {
    throw std::invalid_argument(
        "Alias canonical node must be a table or array: " +
        replacement.old_canonical);
  }
  source_parent->erase(source_key);
  if (kIsGroup) {
    source_parent->insert(kNewKey, std::move(group_copy));
  } else {
    source_parent->insert(kNewKey, std::move(aliases_copy));
  }
}

using ActivityHierarchyDocument =
    tracer::core::infrastructure::config::loader::detail::ActivityHierarchyDocument;
using ActivityHierarchyCanonicalNode = tracer::core::infrastructure::config::
    loader::detail::ActivityHierarchyCanonicalNode;
using ActivityHierarchyDocumentNodeKind =
    tracer::core::infrastructure::config::loader::detail::ActivityHierarchyDocumentNodeKind;

auto ParseHierarchyPath(std::string_view value, bool allow_root,
                        std::string_view field) -> std::vector<std::string> {
  if (allow_root && value == "root") {
    return {};
  }
  if (value.empty()) {
    throw std::invalid_argument(std::string(field) + " must not be empty.");
  }
  return SplitPath(value);
}

auto PathsEqual(const std::vector<std::string>& left,
                const std::vector<std::string>& right) -> bool {
  return left.size() == right.size() &&
         std::equal(left.begin(), left.end(), right.begin());
}

auto RequireCanonicalNode(const ActivityHierarchyDocument& document,
                          const std::vector<std::string>& path,
                          ActivityHierarchyDocumentNodeKind kind,
                          std::string_view description)
    -> ActivityHierarchyCanonicalNode {
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectActivityHierarchyCanonicalNodes(document)) {
    if (node.kind == kind && PathsEqual(node.path, path)) {
      return node;
    }
  }
  throw std::invalid_argument(
      "Alias " + std::string(description) + " not found: " + [&path]() {
        std::ostringstream output;
        for (std::size_t index = 0U; index < path.size(); ++index) {
          if (index != 0U) {
            output << '.';
          }
          output << path[index];
        }
        return output.str();
      }());
}

auto ResolveLeafPath(const ActivityHierarchyDocument& document,
                     const config::ActivityHierarchyOperationRequest& request)
    -> std::vector<std::string> {
  if (!request.target_path.empty()) {
    return ParseHierarchyPath(request.target_path, false, "Leaf path");
  }
  if (request.target_alias.empty()) {
    throw std::invalid_argument("Leaf path or target alias must be provided.");
  }
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectActivityHierarchyCanonicalNodes(document)) {
    if (node.kind != ActivityHierarchyDocumentNodeKind::kLeaf) {
      continue;
    }
    for (const auto& alias : node.node->aliases) {
      if (alias.value == request.target_alias) {
        return node.path;
      }
    }
  }
  throw std::invalid_argument("Alias leaf not found: " + request.target_alias);
}

auto ValidateParentName(std::string_view new_name) -> void {
  ValidateNewKey(new_name);
  if (new_name.find('/') != std::string_view::npos ||
      new_name.find('\\') != std::string_view::npos ||
      std::ranges::any_of(new_name, [](unsigned char value) {
        return std::isspace(value) != 0;
      })) {
    throw std::invalid_argument(
        "New parent name must be one path-safe canonical segment: " +
        std::string(new_name));
  }
}

auto ResolveGroupPath(const ActivityHierarchyDocument& document,
                      const config::ActivityHierarchyOperationRequest& request)
    -> std::vector<std::string> {
  if (!request.target_path.empty()) {
    return ParseHierarchyPath(request.target_path, false, "Group path");
  }
  if (request.target_alias.empty()) {
    throw std::invalid_argument("Group path or target alias must be provided.");
  }
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectActivityHierarchyCanonicalNodes(document)) {
    if (node.kind != ActivityHierarchyDocumentNodeKind::kGroup) {
      continue;
    }
    for (const auto& alias : node.node->aliases) {
      if (alias.value == request.target_alias) {
        return node.path;
      }
    }
  }
  throw std::invalid_argument("Alias group not found: " + request.target_alias);
}

[[nodiscard]] auto IsPathPrefix(const std::vector<std::string>& prefix,
                                const std::vector<std::string>& path) -> bool {
  return path.size() >= prefix.size() &&
         std::equal(prefix.begin(), prefix.end(), path.begin());
}

[[nodiscard]] auto BuildNodeCanonical(std::string_view parent,
                                      const std::vector<std::string>& path,
                                      ActivityHierarchyDocumentNodeKind kind)
    -> std::string {
  if (kind == ActivityHierarchyDocumentNodeKind::kGroup) {
    return tracer::core::infrastructure::config::loader::detail::
        BuildActivityHierarchyCanonicalPath(parent, path);
  }
  return tracer::core::infrastructure::config::loader::detail::
      BuildActivityHierarchyCanonicalPath(
          parent, std::vector<std::string>(path.begin(), path.end() - 1),
          path.back());
}

auto MakeAliasArray(const std::vector<std::string>& aliases) -> toml::array {
  if (aliases.empty()) {
    throw std::invalid_argument("Alias list must not be empty.");
  }
  toml::array result;
  for (const auto& alias : aliases) {
    if (alias.empty()) {
      throw std::invalid_argument("Aliases must not be empty.");
    }
    result.push_back(alias);
  }
  return result;
}

auto SerializeActivityHierarchyDocument(const toml::table& document) -> std::string {
  std::ostringstream output;
  output << document;
  return output.str();
}

auto AppendRawDocumentMigrationPlan(
    const ActivityHierarchyDocument& original, const ActivityHierarchyDocument& updated,
    const std::vector<std::string>& original_groups,
    const std::vector<std::string>& updated_groups,
    std::vector<Replacement>& canonical_replacements,
    std::vector<config::AliasKeyReplacement>& alias_replacements) -> void {
  const auto kCount = std::min(original.nodes.size(), updated.nodes.size());
  for (std::size_t index = 0U; index < kCount; ++index) {
    const auto& old_node = original.nodes[index];
    const auto& new_node = updated.nodes[index];
    if (old_node.kind != new_node.kind) {
      continue;
    }

    const auto kOldPath = tracer::core::infrastructure::config::loader::detail::
        BuildActivityHierarchyCanonicalPath(original.parent, original_groups,
                                old_node.canonical_key);
    const auto kNewPath = tracer::core::infrastructure::config::loader::detail::
        BuildActivityHierarchyCanonicalPath(updated.parent, updated_groups,
                                new_node.canonical_key);
    if (kOldPath != kNewPath) {
      canonical_replacements.push_back({kOldPath, kNewPath});
    }

    const auto kAliasCount =
        std::min(old_node.aliases.size(), new_node.aliases.size());
    for (std::size_t alias_index = 0U; alias_index < kAliasCount;
         ++alias_index) {
      if (old_node.aliases[alias_index].value !=
          new_node.aliases[alias_index].value) {
        alias_replacements.push_back({old_node.aliases[alias_index].value,
                                      new_node.aliases[alias_index].value});
      }
    }

    if (old_node.kind == ActivityHierarchyDocumentNodeKind::kGroup) {
      auto old_child_groups = original_groups;
      old_child_groups.push_back(old_node.canonical_key);
      auto new_child_groups = updated_groups;
      new_child_groups.push_back(new_node.canonical_key);
      AppendRawDocumentMigrationPlan(
          ActivityHierarchyDocument{.parent = original.parent, .nodes = old_node.children},
          ActivityHierarchyDocument{.parent = updated.parent, .nodes = new_node.children},
          old_child_groups, new_child_groups, canonical_replacements,
          alias_replacements);
    }
  }
}

auto DescribeNode(const tracer::core::infrastructure::config::loader::detail::
                      ActivityHierarchyNode& node,
                  const std::vector<std::string>& parent_path)
    -> config::ActivityHierarchyNodeSnapshot {
  auto path = parent_path;
  path.push_back(node.canonical_key);
  std::ostringstream path_text;
  for (std::size_t index = 0U; index < path.size(); ++index) {
    if (index != 0U) {
      path_text << '.';
    }
    path_text << path[index];
  }
  config::ActivityHierarchyNodeSnapshot snapshot{
      .canonical_key = node.canonical_key,
      .path = path_text.str(),
      .kind = node.kind == ActivityHierarchyDocumentNodeKind::kGroup
                  ? config::ActivityHierarchyNodeKind::kGroup
                  : config::ActivityHierarchyNodeKind::kLeaf,
  };
  for (const auto& alias : node.aliases) {
    snapshot.aliases.push_back(alias.value);
  }
  for (const auto& child : node.children) {
    snapshot.children.push_back(DescribeNode(child, path));
  }
  return snapshot;
}

auto CollectRenamedCanonicalReplacements(
    const ActivityHierarchyDocument& document, const std::vector<std::string>& target_path,
    std::string_view new_name) -> std::vector<Replacement> {
  std::vector<Replacement> replacements;
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectActivityHierarchyCanonicalNodes(document)) {
    if (node.path.size() < target_path.size() ||
        !std::equal(target_path.begin(), target_path.end(),
                    node.path.begin())) {
      continue;
    }
    auto renamed_path = node.path;
    renamed_path[target_path.size() - 1U] = std::string(new_name);
    const std::vector<std::string> kGroups =
        node.kind == ActivityHierarchyDocumentNodeKind::kGroup
            ? renamed_path
            : std::vector<std::string>(renamed_path.begin(),
                                       renamed_path.end() - 1);
    replacements.push_back(
        {node.canonical,
         tracer::core::infrastructure::config::loader::detail::
             BuildActivityHierarchyCanonicalPath(document.parent, kGroups,
                                     node.kind == ActivityHierarchyDocumentNodeKind::kGroup
                                         ? std::string_view{}
                                         : renamed_path.back())});
  }
  return replacements;
}

auto ApplyActivityHierarchyOperationImpl(
    std::string_view toml_content,
    const config::ActivityHierarchyOperationRequest& request)
    -> config::ActivityHierarchyOperationResult {
  if (toml_content.empty()) {
    throw std::invalid_argument("Canonical TOML content must not be empty.");
  }

  toml::table document = toml::parse(toml_content);
  ActivityHierarchyDocument activity_hierarchy_document =
      tracer::core::infrastructure::config::loader::detail::ParseActivityHierarchyDocument(
          document);
  tracer::core::infrastructure::config::loader::detail::
      ValidateActivityHierarchyAliasUniqueness(activity_hierarchy_document);
  toml::table* canonical = document["canonical"].as_table();
  config::ActivityHierarchyOperationResult result;

  const auto kCollectAliasReplacements = [&]() {
    if (request.aliases.empty() ||
        (request.kind !=
             config::ActivityHierarchyOperationKind::kSetLeafAliases &&
         request.kind !=
             config::ActivityHierarchyOperationKind::kRenameLeafCanonical)) {
      return;
    }
    const auto kPath =
        ParseHierarchyPath(request.target_path, false, "Leaf path");
    const auto& current = RequireCanonicalNode(
        activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kLeaf, "leaf");
    if (current.node->aliases.size() != request.aliases.size()) {
      return;
    }
    for (std::size_t index = 0U; index < current.node->aliases.size();
         ++index) {
      if (current.node->aliases[index].value != request.aliases[index]) {
        result.alias_replacements.push_back(
            {.old_alias = current.node->aliases[index].value,
             .new_alias = request.aliases[index]});
      }
    }
  };
  kCollectAliasReplacements();

  const auto kCollectGroupAliasReplacements = [&]() {
    if (request.kind !=
        config::ActivityHierarchyOperationKind::kSetGroupAliases) {
      return;
    }
    const auto kPath =
        ParseHierarchyPath(request.target_path, false, "Group path");
    const auto& current = RequireCanonicalNode(
        activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kGroup, "group");
    if (current.node->aliases.size() != request.aliases.size()) {
      return;
    }
    for (std::size_t index = 0U; index < current.node->aliases.size();
         ++index) {
      if (current.node->aliases[index].value != request.aliases[index]) {
        result.alias_replacements.push_back(
            {.old_alias = current.node->aliases[index].value,
             .new_alias = request.aliases[index]});
      }
    }
  };
  kCollectGroupAliasReplacements();

  const auto kMutableParent =
      [&](const std::vector<std::string>& path) -> toml::table* {
    toml::table* parent = FindTable(*canonical, path);
    if (parent == nullptr) {
      throw std::invalid_argument("Alias group not found.");
    }
    return parent;
  };

  switch (request.kind) {
    case config::ActivityHierarchyOperationKind::kAddGroup: {
      const auto kParentPath =
          ParseHierarchyPath(request.target_path, true, "Parent group path");
      ValidateNewKey(request.canonical_key);
      toml::table* parent = kMutableParent(kParentPath);
      if (parent->contains(request.canonical_key)) {
        throw std::invalid_argument("Canonical key already exists: " +
                                    request.canonical_key);
      }
      parent->insert(request.canonical_key, toml::table{});
      break;
    }
    case config::ActivityHierarchyOperationKind::kDeleteGroup: {
      const auto kPath =
          ParseHierarchyPath(request.target_path, false, "Group path");
      static_cast<void>(RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kGroup, "group"));
      auto parent_path = kPath;
      const std::string kKey = parent_path.back();
      parent_path.pop_back();
      kMutableParent(parent_path)->erase(kKey);
      break;
    }
    case config::ActivityHierarchyOperationKind::kAddLeaf: {
      const auto kParentPath =
          ParseHierarchyPath(request.target_path, true, "Parent group path");
      ValidateNewKey(request.canonical_key);
      toml::table* parent = kMutableParent(kParentPath);
      if (parent->contains(request.canonical_key)) {
        throw std::invalid_argument("Canonical key already exists: " +
                                    request.canonical_key);
      }
      parent->insert(request.canonical_key, MakeAliasArray(request.aliases));
      break;
    }
    case config::ActivityHierarchyOperationKind::kSetLeafAliases: {
      const auto kPath =
          ParseHierarchyPath(request.target_path, false, "Leaf path");
      static_cast<void>(RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kLeaf, "leaf"));
      auto parent_path = kPath;
      const std::string kKey = parent_path.back();
      parent_path.pop_back();
      toml::table* parent = kMutableParent(parent_path);
      parent->erase(kKey);
      parent->insert(kKey, MakeAliasArray(request.aliases));
      break;
    }
    case config::ActivityHierarchyOperationKind::kDeleteLeaf: {
      const auto kPath =
          ParseHierarchyPath(request.target_path, false, "Leaf path");
      static_cast<void>(RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kLeaf, "leaf"));
      auto parent_path = kPath;
      const std::string kKey = parent_path.back();
      parent_path.pop_back();
      kMutableParent(parent_path)->erase(kKey);
      break;
    }
    case config::ActivityHierarchyOperationKind::kPromoteLeaf: {
      const auto kPath = ResolveLeafPath(activity_hierarchy_document, request);
      const auto kSource = RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kLeaf, "leaf");
      auto parent_path = kPath;
      const std::string kKey = parent_path.back();
      parent_path.pop_back();
      toml::table* parent = kMutableParent(parent_path);
      toml::table group;
      toml::array group_aliases;
      for (const auto& alias : kSource.node->aliases) {
        group_aliases.push_back(alias.value);
      }
      group.insert("group_aliases", std::move(group_aliases));
      parent->erase(kKey);
      parent->insert(kKey, std::move(group));
      break;
    }
    case config::ActivityHierarchyOperationKind::kMoveLeaf: {
      const auto kSourcePath = ResolveLeafPath(activity_hierarchy_document, request);
      const auto kDestinationPath = ParseHierarchyPath(
          request.destination_path, false, "Destination group path");
      const auto kSource = RequireCanonicalNode(
          activity_hierarchy_document, kSourcePath, ActivityHierarchyDocumentNodeKind::kLeaf, "leaf");
      static_cast<void>(RequireCanonicalNode(activity_hierarchy_document, kDestinationPath,
                                             ActivityHierarchyDocumentNodeKind::kGroup,
                                             "destination group"));
      auto source_parent_path = kSourcePath;
      const std::string kKey = source_parent_path.back();
      source_parent_path.pop_back();
      if (PathsEqual(source_parent_path, kDestinationPath)) {
        throw std::invalid_argument(
            "Canonical leaf is already in the target group.");
      }
      toml::table* destination = kMutableParent(kDestinationPath);
      if (destination->contains(kKey)) {
        throw std::invalid_argument("Canonical key already exists: " + kKey);
      }
      toml::array values;
      for (const auto& alias : kSource.node->aliases) {
        values.push_back(alias.value);
      }
      kMutableParent(source_parent_path)->erase(kKey);
      destination->insert(kKey, std::move(values));
      result.replacements.push_back({
          kSource.canonical,
          tracer::core::infrastructure::config::loader::detail::
              BuildActivityHierarchyCanonicalPath(activity_hierarchy_document.parent, kDestinationPath,
                                      kKey),
      });
      break;
    }
    case config::ActivityHierarchyOperationKind::kMoveGroup: {
      const auto kSourcePath = ResolveGroupPath(activity_hierarchy_document, request);
      const auto kDestinationPath = request.destination_path == "root"
                                        ? std::vector<std::string>{}
                                        : ParseHierarchyPath(
                                              request.destination_path, false,
                                              "Destination group path");
      const auto kSource = RequireCanonicalNode(
          activity_hierarchy_document, kSourcePath, ActivityHierarchyDocumentNodeKind::kGroup, "group");
      if (!kDestinationPath.empty()) {
        static_cast<void>(RequireCanonicalNode(
            activity_hierarchy_document, kDestinationPath, ActivityHierarchyDocumentNodeKind::kGroup,
            "destination group"));
      }
      if (IsPathPrefix(kSourcePath, kDestinationPath)) {
        throw std::invalid_argument(
            "A group cannot be moved into itself or its descendants.");
      }

      auto source_parent_path = kSourcePath;
      const std::string kKey = source_parent_path.back();
      source_parent_path.pop_back();
      toml::table* destination = kMutableParent(kDestinationPath);
      if (destination->contains(kKey)) {
        throw std::invalid_argument("Canonical key already exists: " + kKey);
      }

      toml::table moved_group = *kMutableParent(source_parent_path)
                                     ->at(kKey)
                                     .as_table();
      kMutableParent(source_parent_path)->erase(kKey);
      destination->insert(kKey, std::move(moved_group));

      for (const auto& node : tracer::core::infrastructure::config::loader::
               detail::CollectActivityHierarchyCanonicalNodes(activity_hierarchy_document)) {
        if (!IsPathPrefix(kSourcePath, node.path)) {
          continue;
        }
        auto new_path = kDestinationPath;
        new_path.push_back(kKey);
        new_path.insert(
            new_path.end(),
            node.path.begin() + static_cast<
                std::vector<std::string>::difference_type>(kSourcePath.size()),
            node.path.end());
        const auto kNewCanonical = BuildNodeCanonical(
            activity_hierarchy_document.parent, new_path, node.kind);
        if (node.canonical != kNewCanonical) {
          result.replacements.push_back({node.canonical, kNewCanonical});
        }
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kMergeLeafCanonical: {
      const auto kSourcePath =
          ParseHierarchyPath(request.target_path, false, "Source leaf path");
      const auto kDestinationPath = ParseHierarchyPath(
          request.destination_path, false, "Destination leaf path");
      if (PathsEqual(kSourcePath, kDestinationPath)) {
        throw std::invalid_argument(
            "Source and destination canonical leaves must differ.");
      }

      const auto kSource =
          RequireCanonicalNode(activity_hierarchy_document, kSourcePath,
                               ActivityHierarchyDocumentNodeKind::kLeaf, "source leaf");
      const auto kDestination = RequireCanonicalNode(
          activity_hierarchy_document, kDestinationPath, ActivityHierarchyDocumentNodeKind::kLeaf,
          "destination leaf");
      if (kDestination.node->aliases.empty()) {
        throw std::invalid_argument(
            "Destination canonical leaf must contain at least one alias.");
      }

      auto source_parent_path = kSourcePath;
      const std::string kSourceKey = source_parent_path.back();
      source_parent_path.pop_back();
      kMutableParent(source_parent_path)->erase(kSourceKey);

      result.replacements.push_back({kSource.canonical, kDestination.canonical});
      for (const auto& alias : kSource.node->aliases) {
        result.alias_replacements.push_back(
            {alias.value, kDestination.node->aliases.front().value});
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kSetGroupAliases: {
      const auto kPath =
          ParseHierarchyPath(request.target_path, false, "Group path");
      static_cast<void>(RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kGroup, "group"));
      toml::table* group = kMutableParent(kPath);
      group->erase("group_aliases");
      if (!request.aliases.empty()) {
        group->insert("group_aliases", MakeAliasArray(request.aliases));
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kRenameParent: {
      if (!request.old_parent.empty() &&
          request.old_parent != activity_hierarchy_document.parent) {
        throw std::invalid_argument(
            "Old parent does not match the canonical TOML parent: " +
            request.old_parent + " != " + activity_hierarchy_document.parent);
      }
      ValidateParentName(request.new_name);
      if (request.new_name == activity_hierarchy_document.parent) {
        throw std::invalid_argument(
            "New parent name must differ from the current parent.");
      }
      for (const auto& node : tracer::core::infrastructure::config::loader::
               detail::CollectActivityHierarchyCanonicalNodes(activity_hierarchy_document)) {
        result.replacements.push_back(
            {node.canonical,
             tracer::core::infrastructure::config::loader::detail::
                 BuildActivityHierarchyCanonicalPath(
                     request.new_name,
                     node.kind == ActivityHierarchyDocumentNodeKind::kGroup
                         ? node.path
                         : std::vector<std::string>(node.path.begin(),
                                                    node.path.end() - 1),
                     node.kind == ActivityHierarchyDocumentNodeKind::kGroup
                         ? std::string_view{}
                         : node.path.back())});
      }
      document.erase("parent");
      document.insert("parent", request.new_name);
      break;
    }
    case config::ActivityHierarchyOperationKind::kSetParentColor: {
      document.erase("color");
      if (request.color.has_value()) {
        if (!tracer::core::infrastructure::config::loader::detail::
                IsActivityHierarchyParentColor(*request.color)) {
          throw std::invalid_argument(
              "Parent color must be an uppercase or lowercase #RRGGBB string.");
        }
        document.insert("color", *request.color);
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kRenameGroupCanonical:
    case config::ActivityHierarchyOperationKind::kRenameLeafCanonical: {
      const bool kGroup =
          request.kind ==
          config::ActivityHierarchyOperationKind::kRenameGroupCanonical;
      const auto kPath = ParseHierarchyPath(request.target_path, false,
                                           kGroup ? "Group path" : "Leaf path");
      ValidateNewKey(request.new_name);
      const auto kSource = RequireCanonicalNode(
          activity_hierarchy_document, kPath,
          kGroup ? ActivityHierarchyDocumentNodeKind::kGroup : ActivityHierarchyDocumentNodeKind::kLeaf,
          kGroup ? "group" : "leaf");
      auto parent_path = kPath;
      parent_path.pop_back();
      const std::string kNewCanonical = tracer::core::infrastructure::config::
          loader::detail::BuildActivityHierarchyCanonicalPath(
              activity_hierarchy_document.parent, parent_path, request.new_name);
      result.replacements = CollectRenamedCanonicalReplacements(
          activity_hierarchy_document, kPath, request.new_name);
      RenameOne({kSource.canonical, kNewCanonical}, *canonical, activity_hierarchy_document);
      if (!kGroup && !request.aliases.empty()) {
        toml::table* parent = kMutableParent(parent_path);
        parent->erase(request.new_name);
        parent->insert(request.new_name, MakeAliasArray(request.aliases));
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kAppendLeafAlias: {
      const auto kParentPath =
          ParseHierarchyPath(request.target_path, true, "Parent group path");
      ValidateNewKey(request.canonical_key);
      const auto kAppended = MakeAliasArray(request.aliases);
      toml::table* parent = kMutableParent(kParentPath);
      if (!parent->contains(request.canonical_key)) {
        parent->insert(request.canonical_key, std::move(kAppended));
        break;
      }
      toml::node& leaf = parent->at(request.canonical_key);
      auto* existing = leaf.as_array();
      if (existing == nullptr) {
        throw std::invalid_argument(
            "Canonical key is already used as a group: " +
            request.canonical_key);
      }
      for (const auto& alias : request.aliases) {
        existing->push_back(alias);
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kAppendGroupAlias: {
      const auto kPath =
          ParseHierarchyPath(request.target_path, false, "Group path");
      static_cast<void>(RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kGroup, "group"));
      const auto kAppended = MakeAliasArray(request.aliases);
      toml::table* group = kMutableParent(kPath);
      auto* existing = group->get_as<toml::array>("group_aliases");
      if (existing == nullptr) {
        group->insert("group_aliases", std::move(kAppended));
      } else {
        for (const auto& alias : request.aliases) {
          existing->push_back(alias);
        }
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kRenameGroupAlias: {
      const auto kPath =
          ParseHierarchyPath(request.target_path, false, "Group path");
      if (request.old_alias.empty() || request.new_name.empty()) {
        throw std::invalid_argument(
            "Old and new group aliases must not be empty.");
      }
      static_cast<void>(RequireCanonicalNode(
          activity_hierarchy_document, kPath, ActivityHierarchyDocumentNodeKind::kGroup, "group"));
      toml::table* group = kMutableParent(kPath);
      auto* existing = group->get_as<toml::array>("group_aliases");
      if (existing == nullptr) {
        throw std::invalid_argument("Group alias not found: " +
                                    request.old_alias);
      }
      bool replaced = false;
      for (auto alias = existing->cbegin(); alias != existing->cend();
           ++alias) {
        if (alias->value<std::string>() == request.old_alias) {
          existing->replace(alias, request.new_name);
          replaced = true;
          break;
        }
      }
      if (!replaced) {
        throw std::invalid_argument("Group alias not found: " +
                                    request.old_alias);
      }
      result.alias_replacements.push_back(
          {request.old_alias, request.new_name});
      break;
    }
  }

  const ActivityHierarchyDocument kUpdated =
      tracer::core::infrastructure::config::loader::detail::ParseActivityHierarchyDocument(
          document);
  tracer::core::infrastructure::config::loader::detail::
      ValidateActivityHierarchyAliasUniqueness(kUpdated);
  result.updated_toml_content = SerializeActivityHierarchyDocument(document);
  return result;
}

}  // namespace

namespace tracer::core::application::config {

auto ApplyActivityHierarchyOperation(
    std::string_view toml_content,
    const ActivityHierarchyOperationRequest& request)
    -> ActivityHierarchyOperationResult {
  return ApplyActivityHierarchyOperationImpl(toml_content, request);
}

auto MoveActivityHierarchyNodeBetweenDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents,
    std::string_view source_name, std::string_view destination_name,
    const ActivityHierarchyOperationRequest& request)
    -> ActivityHierarchyCrossDocumentOperationResult {
  if (documents.empty()) {
    throw std::invalid_argument(
        "Canonical TOML document set must not be empty.");
  }
  if (source_name.empty() || destination_name.empty()) {
    throw std::invalid_argument(
        "Source and destination canonical TOML names must not be empty.");
  }
  if (source_name == destination_name) {
    throw std::invalid_argument(
        "Cross-document leaf move requires different source and destination "
        "documents.");
  }

  ValidateActivityHierarchyDocuments(documents);

  std::size_t source_index = documents.size();
  std::size_t destination_index = documents.size();
  std::vector<toml::table> tables;
  tables.reserve(documents.size());
  std::vector<ActivityHierarchyDocument> parsed_documents;
  parsed_documents.reserve(documents.size());
  for (std::size_t index = 0U; index < documents.size(); ++index) {
    if (documents[index].source_name == source_name) {
      if (source_index != documents.size()) {
        throw std::invalid_argument("Duplicate source canonical TOML name: " +
                                    std::string(source_name));
      }
      source_index = index;
    }
    if (documents[index].source_name == destination_name) {
      if (destination_index != documents.size()) {
        throw std::invalid_argument(
            "Duplicate destination canonical TOML name: " +
            std::string(destination_name));
      }
      destination_index = index;
    }
    tables.push_back(toml::parse(documents[index].toml_content));
    parsed_documents.push_back(
        infrastructure::config::loader::detail::ParseActivityHierarchyDocument(
            tables.back()));
  }
  if (source_index == documents.size()) {
    throw std::invalid_argument("Source canonical TOML not found: " +
                                std::string(source_name));
  }
  if (destination_index == documents.size()) {
    throw std::invalid_argument("Destination canonical TOML not found: " +
                                std::string(destination_name));
  }

  if (request.kind != config::ActivityHierarchyOperationKind::kMoveLeaf &&
      request.kind != config::ActivityHierarchyOperationKind::kMoveGroup) {
    throw std::invalid_argument(
        "Cross-document move supports only move_leaf and move_group.");
  }
  const auto kSourcePath =
      request.kind == config::ActivityHierarchyOperationKind::kMoveLeaf
          ? ResolveLeafPath(parsed_documents[source_index], request)
          : ResolveGroupPath(parsed_documents[source_index], request);
  const auto kSource = RequireCanonicalNode(
      parsed_documents[source_index], kSourcePath,
      request.kind == config::ActivityHierarchyOperationKind::kMoveLeaf
          ? ActivityHierarchyDocumentNodeKind::kLeaf
          : ActivityHierarchyDocumentNodeKind::kGroup,
      request.kind == config::ActivityHierarchyOperationKind::kMoveLeaf
          ? "leaf"
          : "group");
  auto source_parent_path = kSourcePath;
  const std::string kSourceKey = source_parent_path.back();
  source_parent_path.pop_back();

  std::vector<std::string> destination_path;
  if (request.destination_path == "root") {
    destination_path = {};
  } else {
    destination_path = ParseHierarchyPath(request.destination_path, false,
                                          "Destination group path");
    static_cast<void>(RequireCanonicalNode(
        parsed_documents[destination_index], destination_path,
        ActivityHierarchyDocumentNodeKind::kGroup, "destination group"));
  }

  toml::table* source_canonical = tables[source_index]["canonical"].as_table();
  toml::table* destination_canonical =
      tables[destination_index]["canonical"].as_table();
  if (source_canonical == nullptr || destination_canonical == nullptr) {
    throw std::invalid_argument(
        "Canonical TOML must contain a `canonical` table.");
  }
  toml::table* source_parent = FindTable(*source_canonical, source_parent_path);
  toml::table* destination_parent =
      FindTable(*destination_canonical, destination_path);
  if (source_parent == nullptr || destination_parent == nullptr) {
    throw std::invalid_argument("Alias group not found.");
  }
  if (destination_parent->contains(kSourceKey)) {
    throw std::invalid_argument(
        "Canonical key already exists in destination: " + kSourceKey);
  }

  toml::table moved_group;
  toml::array moved_values;
  if (request.kind == config::ActivityHierarchyOperationKind::kMoveGroup) {
    moved_group = *source_parent->at(kSourceKey).as_table();
  } else {
    for (const auto& alias : kSource.node->aliases) {
      moved_values.push_back(alias.value);
    }
  }
  source_parent->erase(kSourceKey);
  if (request.kind == config::ActivityHierarchyOperationKind::kMoveGroup) {
    destination_parent->insert(kSourceKey, std::move(moved_group));
  } else {
    destination_parent->insert(kSourceKey, std::move(moved_values));
  }

  std::vector<AliasCanonicalReplacement> replacements;
  for (const auto& node : infrastructure::config::loader::detail::
           CollectActivityHierarchyCanonicalNodes(parsed_documents[source_index])) {
    if (!IsPathPrefix(kSourcePath, node.path)) {
      continue;
    }
    auto new_path = destination_path;
    new_path.push_back(kSourceKey);
    new_path.insert(
        new_path.end(),
        node.path.begin() + static_cast<std::vector<std::string>::difference_type>(
                                kSourcePath.size()),
                    node.path.end());
    const auto kOldCanonical = BuildNodeCanonical(
        parsed_documents[source_index].parent, node.path, node.kind);
    const auto kNewCanonical = BuildNodeCanonical(
        parsed_documents[destination_index].parent, new_path, node.kind);
    if (kOldCanonical != kNewCanonical) {
      replacements.push_back({kOldCanonical, kNewCanonical});
    }
  }

  std::vector<ActivityHierarchyDocumentInput> updated_documents;
  updated_documents.reserve(documents.size());
  for (std::size_t index = 0U; index < documents.size(); ++index) {
    updated_documents.push_back(
        {documents[index].source_name, SerializeActivityHierarchyDocument(tables[index])});
  }
  ValidateActivityHierarchyDocuments(updated_documents);

  ActivityHierarchyCrossDocumentOperationResult result;
  result.updated_documents.push_back(
      {std::string(source_name), updated_documents[source_index].toml_content});
  result.updated_documents.push_back(
      {std::string(destination_name),
       updated_documents[destination_index].toml_content});
  result.replacements = std::move(replacements);
  return result;
}

auto MoveActivityHierarchyLeafBetweenDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents,
    std::string_view source_name, std::string_view destination_name,
    const ActivityHierarchyOperationRequest& request)
    -> ActivityHierarchyCrossDocumentOperationResult {
  if (request.kind != ActivityHierarchyOperationKind::kMoveLeaf) {
    throw std::invalid_argument(
        "Leaf cross-document move requires the move_leaf operation.");
  }
  return MoveActivityHierarchyNodeBetweenDocuments(documents, source_name,
                                                    destination_name, request);
}

auto RewriteActivityHierarchyDocument(std::string_view original_toml_content,
                                      std::string_view updated_toml_content)
    -> ActivityHierarchyOperationResult {
  if (original_toml_content.empty() || updated_toml_content.empty()) {
    throw std::invalid_argument("Canonical TOML content must not be empty.");
  }
  const toml::table kOriginalTable = toml::parse(original_toml_content);
  const toml::table kUpdatedTable = toml::parse(updated_toml_content);
  const ActivityHierarchyDocument kOriginal =
      infrastructure::config::loader::detail::ParseActivityHierarchyDocument(
          kOriginalTable);
  const ActivityHierarchyDocument kUpdated =
      infrastructure::config::loader::detail::ParseActivityHierarchyDocument(kUpdatedTable);
  infrastructure::config::loader::detail::ValidateActivityHierarchyAliasUniqueness(
      kOriginal);
  infrastructure::config::loader::detail::ValidateActivityHierarchyAliasUniqueness(
      kUpdated);

  ActivityHierarchyOperationResult result;
  AppendRawDocumentMigrationPlan(kOriginal, kUpdated, {}, {}, result.replacements,
                                 result.alias_replacements);
  result.updated_toml_content = SerializeActivityHierarchyDocument(kUpdatedTable);
  return result;
}

auto DescribeActivityHierarchy(std::string_view toml_content)
    -> ActivityHierarchySnapshot {
  const toml::table kParsed = toml::parse(toml_content);
  const ActivityHierarchyDocument kDocument =
      infrastructure::config::loader::detail::ParseActivityHierarchyDocument(kParsed);
  infrastructure::config::loader::detail::ValidateActivityHierarchyAliasUniqueness(
      kDocument);
  ActivityHierarchySnapshot snapshot{.parent = kDocument.parent,
                                     .color = kDocument.color};
  for (const auto& node : kDocument.nodes) {
    snapshot.nodes.push_back(DescribeNode(node, {}));
  }
  return snapshot;
}

auto ValidateActivityHierarchyDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents) -> void {
  std::map<std::string, std::string> alias_sources;
  for (const auto& input : documents) {
    if (input.source_name.empty()) {
      throw std::invalid_argument(
          "Canonical document source name must not be empty.");
    }
    const toml::table kParsed = toml::parse(input.toml_content);
    const ActivityHierarchyDocument kDocument =
        infrastructure::config::loader::detail::ParseActivityHierarchyDocument(kParsed);
    infrastructure::config::loader::detail::
        ValidateActivityHierarchyAliasUniqueness(kDocument);
    for (const auto& node : infrastructure::config::loader::detail::
             CollectActivityHierarchyCanonicalNodes(kDocument)) {
      for (const auto& alias : node.node->aliases) {
        const auto [existing, inserted] =
            alias_sources.emplace(alias.value, input.source_name);
        if (!inserted) {
          throw std::runtime_error(
              "Duplicate alias key `" + alias.value +
              "` across canonical documents: " + existing->second + " and " +
              input.source_name + ".");
        }
      }
    }
  }
}

}  // namespace tracer::core::application::config

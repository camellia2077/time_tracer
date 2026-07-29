#include "application/ports/config/alias_toml_editor.hpp"

#include "infra/config/loader/alias_document.hpp"
#include "infra/config/loader/alias_toml_editor.hpp"

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
    const tracer::core::infrastructure::config::loader::detail::AliasDocument&
        document,
    std::map<std::string, Match>& matches) -> void {
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectAliasDocumentCanonicalNodes(document)) {
    matches.emplace(node.canonical,
                    Match{node.canonical, node.path,
                          node.kind == tracer::core::infrastructure::config::
                                           loader::detail::AliasDocumentNodeKind::
                                               kGroup});
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
    const auto end = path.find('.', start);
    const auto part = path.substr(start, end == std::string_view::npos
                                           ? path.size() - start
                                           : end - start);
    if (part.empty()) {
      throw std::invalid_argument("Canonical target path contains an empty component.");
    }
    parts.emplace_back(part);
    if (end == std::string_view::npos) {
      break;
    }
    start = end + 1;
  }
  return parts;
}

[[nodiscard]] auto FindTable(toml::table& aliases,
                             const std::vector<std::string>& groups)
    -> toml::table* {
  toml::table* current = &aliases;
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
      new_name.find('.') != std::string_view::npos ||
      new_name.find('_') == 0) {
    throw std::invalid_argument("New canonical key is invalid: " +
                                std::string(new_name));
  }
}

auto RenameOne(
    const Replacement& replacement, toml::table& aliases,
    const tracer::core::infrastructure::config::loader::detail::AliasDocument&
        document) -> void {
  std::map<std::string, Match> matches;
  CollectMatches(document, matches);
  const auto source = matches.find(replacement.old_canonical);
  if (source == matches.end()) {
    throw std::invalid_argument("Canonical not found: " +
                                replacement.old_canonical);
  }
  if (matches.contains(replacement.new_canonical)) {
    throw std::invalid_argument("Canonical already exists: " +
                                replacement.new_canonical);
  }

  const auto separator = replacement.new_canonical.rfind('_');
  const std::string prefix =
      separator == std::string::npos
          ? std::string{}
          : replacement.new_canonical.substr(0, separator);
  const std::string expected_prefix = source->second.canonical.substr(
      0, source->second.canonical.rfind('_'));
  if (prefix != expected_prefix) {
    throw std::invalid_argument(
        "Canonical rename must keep the node at the same hierarchy level: " +
        replacement.old_canonical + " -> " + replacement.new_canonical);
  }
  const std::string new_key =
      separator == std::string::npos
          ? replacement.new_canonical
          : replacement.new_canonical.substr(separator + 1);
  if (new_key.empty() || new_key == "group_aliases") {
    throw std::invalid_argument("New canonical key is invalid: " + new_key);
  }

  std::vector<std::string> parent_path = source->second.path;
  parent_path.pop_back();
  toml::table* source_parent = FindTable(aliases, parent_path);
  if (source_parent == nullptr) {
    throw std::invalid_argument("Canonical not found: " +
                                replacement.old_canonical);
  }

  const std::string& source_key = source->second.path.back();
  toml::table group_copy;
  toml::array aliases_copy;
  toml::node& source_node = source_parent->at(source_key);
  const bool is_group = source_node.is_table();
  if (is_group) {
    group_copy = *source_node.as_table();
  } else if (source_node.is_array()) {
    aliases_copy = *source_node.as_array();
  } else {
    throw std::invalid_argument(
        "Alias canonical node must be a table or array: " +
        replacement.old_canonical);
  }
  source_parent->erase(source_key);
  if (is_group) {
    source_parent->insert(new_key, std::move(group_copy));
  } else {
    source_parent->insert(new_key, std::move(aliases_copy));
  }
}

using AliasDocument =
    tracer::core::infrastructure::config::loader::detail::AliasDocument;
using AliasDocumentCanonicalNode = tracer::core::infrastructure::config::
    loader::detail::AliasDocumentCanonicalNode;
using AliasDocumentNodeKind = tracer::core::infrastructure::config::loader::
    detail::AliasDocumentNodeKind;

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

auto RequireCanonicalNode(const AliasDocument& document,
                          const std::vector<std::string>& path,
                          AliasDocumentNodeKind kind,
                          std::string_view description)
    -> AliasDocumentCanonicalNode {
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectAliasDocumentCanonicalNodes(document)) {
    if (node.kind == kind && PathsEqual(node.path, path)) {
      return node;
    }
  }
  throw std::invalid_argument("Alias " + std::string(description) +
                              " not found: " +
                              [&path]() {
                                std::ostringstream output;
                                for (std::size_t index = 0U; index < path.size();
                                     ++index) {
                                  if (index != 0U) {
                                    output << '.';
                                  }
                                  output << path[index];
                                }
                                return output.str();
                              }());
}

auto ResolveLeafPath(const AliasDocument& document,
                     const config::ActivityHierarchyOperationRequest& request)
    -> std::vector<std::string> {
  if (!request.target_path.empty()) {
    return ParseHierarchyPath(request.target_path, false, "Leaf path");
  }
  if (request.target_alias.empty()) {
    throw std::invalid_argument("Leaf path or target alias must be provided.");
  }
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectAliasDocumentCanonicalNodes(document)) {
    if (node.kind != AliasDocumentNodeKind::kLeaf) {
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

auto ResolveGroupPath(const AliasDocument& document,
                      const config::ActivityHierarchyOperationRequest& request)
    -> std::vector<std::string> {
  if (!request.target_path.empty()) {
    return ParseHierarchyPath(request.target_path, false, "Group path");
  }
  if (request.target_alias.empty()) {
    throw std::invalid_argument("Group path or target alias must be provided.");
  }
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectAliasDocumentCanonicalNodes(document)) {
    if (node.kind != AliasDocumentNodeKind::kGroup) {
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

[[nodiscard]] auto BuildNodeCanonical(
    std::string_view parent, const std::vector<std::string>& path,
    AliasDocumentNodeKind kind) -> std::string {
  if (kind == AliasDocumentNodeKind::kGroup) {
    return tracer::core::infrastructure::config::loader::detail::
        BuildAliasCanonicalPath(parent, path);
  }
  return tracer::core::infrastructure::config::loader::detail::
      BuildAliasCanonicalPath(
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

auto SerializeAliasDocument(const toml::table& document) -> std::string {
  std::ostringstream output;
  output << document;
  return output.str();
}

auto AppendRawDocumentMigrationPlan(
    const AliasDocument& original,
    const AliasDocument& updated,
    const std::vector<std::string>& original_groups,
    const std::vector<std::string>& updated_groups,
    std::vector<Replacement>& canonical_replacements,
    std::vector<config::AliasKeyReplacement>& alias_replacements) -> void {
  const auto count = std::min(original.nodes.size(), updated.nodes.size());
  for (std::size_t index = 0U; index < count; ++index) {
    const auto& old_node = original.nodes[index];
    const auto& new_node = updated.nodes[index];
    if (old_node.kind != new_node.kind) {
      continue;
    }

    const auto old_path = tracer::core::infrastructure::config::loader::detail::
        BuildAliasCanonicalPath(original.parent, original_groups,
                                old_node.canonical_key);
    const auto new_path = tracer::core::infrastructure::config::loader::detail::
        BuildAliasCanonicalPath(updated.parent, updated_groups,
                                new_node.canonical_key);
    if (old_path != new_path) {
      canonical_replacements.push_back({old_path, new_path});
    }

    const auto alias_count = std::min(old_node.aliases.size(),
                                      new_node.aliases.size());
    for (std::size_t alias_index = 0U; alias_index < alias_count;
         ++alias_index) {
      if (old_node.aliases[alias_index].value !=
          new_node.aliases[alias_index].value) {
        alias_replacements.push_back(
            {old_node.aliases[alias_index].value,
             new_node.aliases[alias_index].value});
      }
    }

    if (old_node.kind == AliasDocumentNodeKind::kGroup) {
      auto old_child_groups = original_groups;
      old_child_groups.push_back(old_node.canonical_key);
      auto new_child_groups = updated_groups;
      new_child_groups.push_back(new_node.canonical_key);
      AppendRawDocumentMigrationPlan(
          AliasDocument{.parent = original.parent, .nodes = old_node.children},
          AliasDocument{.parent = updated.parent, .nodes = new_node.children},
          old_child_groups, new_child_groups, canonical_replacements,
          alias_replacements);
    }
  }
}

auto DescribeNode(
    const tracer::core::infrastructure::config::loader::detail::
        AliasDocumentNode& node,
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
      .kind = node.kind == AliasDocumentNodeKind::kGroup
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
    const AliasDocument& document, const std::vector<std::string>& target_path,
    std::string_view new_name) -> std::vector<Replacement> {
  std::vector<Replacement> replacements;
  for (const auto& node : tracer::core::infrastructure::config::loader::detail::
           CollectAliasDocumentCanonicalNodes(document)) {
    if (node.path.size() < target_path.size() ||
        !std::equal(target_path.begin(), target_path.end(), node.path.begin())) {
      continue;
    }
    auto renamed_path = node.path;
    renamed_path[target_path.size() - 1U] = std::string(new_name);
    const std::vector<std::string> groups =
        node.kind == AliasDocumentNodeKind::kGroup
            ? renamed_path
            : std::vector<std::string>(renamed_path.begin(),
                                       renamed_path.end() - 1);
    replacements.push_back(
        {node.canonical,
         tracer::core::infrastructure::config::loader::detail::
             BuildAliasCanonicalPath(document.parent, groups,
                                     node.kind == AliasDocumentNodeKind::kGroup
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
    throw std::invalid_argument("Alias TOML content must not be empty.");
  }

  toml::table document = toml::parse(toml_content);
  AliasDocument alias_document =
      tracer::core::infrastructure::config::loader::detail::ParseAliasDocument(
          document);
  tracer::core::infrastructure::config::loader::detail::
      ValidateAliasDocumentAliasUniqueness(alias_document);
  toml::table* aliases = document["aliases"].as_table();
  config::ActivityHierarchyOperationResult result;

  const auto collect_alias_replacements = [&]() {
    if (request.aliases.empty() ||
        (request.kind != config::ActivityHierarchyOperationKind::kSetLeafAliases &&
         request.kind !=
             config::ActivityHierarchyOperationKind::kRenameLeafCanonical)) {
      return;
    }
    const auto path = ParseHierarchyPath(request.target_path, false, "Leaf path");
    const auto& current = RequireCanonicalNode(
        alias_document, path, AliasDocumentNodeKind::kLeaf, "leaf");
    if (current.node->aliases.size() != request.aliases.size()) {
      return;
    }
    for (std::size_t index = 0U; index < current.node->aliases.size(); ++index) {
      if (current.node->aliases[index].value != request.aliases[index]) {
        result.alias_replacements.push_back(
            {.old_alias = current.node->aliases[index].value,
             .new_alias = request.aliases[index]});
      }
    }
  };
  collect_alias_replacements();

  const auto collect_group_alias_replacements = [&]() {
    if (request.kind != config::ActivityHierarchyOperationKind::kSetGroupAliases) {
      return;
    }
    const auto path = ParseHierarchyPath(request.target_path, false,
                                         "Group path");
    const auto& current = RequireCanonicalNode(
        alias_document, path, AliasDocumentNodeKind::kGroup, "group");
    if (current.node->aliases.size() != request.aliases.size()) {
      return;
    }
    for (std::size_t index = 0U; index < current.node->aliases.size(); ++index) {
      if (current.node->aliases[index].value != request.aliases[index]) {
        result.alias_replacements.push_back(
            {.old_alias = current.node->aliases[index].value,
             .new_alias = request.aliases[index]});
      }
    }
  };
  collect_group_alias_replacements();

  const auto mutable_parent = [&](const std::vector<std::string>& path)
      -> toml::table* {
    toml::table* parent = FindTable(*aliases, path);
    if (parent == nullptr) {
      throw std::invalid_argument("Alias group not found.");
    }
    return parent;
  };

  switch (request.kind) {
    case config::ActivityHierarchyOperationKind::kAddGroup: {
      const auto parent_path =
          ParseHierarchyPath(request.target_path, true, "Parent group path");
      ValidateNewKey(request.canonical_key);
      toml::table* parent = mutable_parent(parent_path);
      if (parent->contains(request.canonical_key)) {
        throw std::invalid_argument("Canonical key already exists: " +
                                    request.canonical_key);
      }
      parent->insert(request.canonical_key, toml::table{});
      break;
    }
    case config::ActivityHierarchyOperationKind::kDeleteGroup: {
      const auto path =
          ParseHierarchyPath(request.target_path, false, "Group path");
      static_cast<void>(RequireCanonicalNode(alias_document, path,
                                             AliasDocumentNodeKind::kGroup,
                                             "group"));
      auto parent_path = path;
      const std::string key = parent_path.back();
      parent_path.pop_back();
      mutable_parent(parent_path)->erase(key);
      break;
    }
    case config::ActivityHierarchyOperationKind::kAddLeaf: {
      const auto parent_path =
          ParseHierarchyPath(request.target_path, true, "Parent group path");
      ValidateNewKey(request.canonical_key);
      toml::table* parent = mutable_parent(parent_path);
      if (parent->contains(request.canonical_key)) {
        throw std::invalid_argument("Canonical key already exists: " +
                                    request.canonical_key);
      }
      parent->insert(request.canonical_key, MakeAliasArray(request.aliases));
      break;
    }
    case config::ActivityHierarchyOperationKind::kSetLeafAliases: {
      const auto path =
          ParseHierarchyPath(request.target_path, false, "Leaf path");
      static_cast<void>(RequireCanonicalNode(alias_document, path,
                                             AliasDocumentNodeKind::kLeaf,
                                             "leaf"));
      auto parent_path = path;
      const std::string key = parent_path.back();
      parent_path.pop_back();
      toml::table* parent = mutable_parent(parent_path);
      parent->erase(key);
      parent->insert(key, MakeAliasArray(request.aliases));
      break;
    }
    case config::ActivityHierarchyOperationKind::kDeleteLeaf: {
      const auto path =
          ParseHierarchyPath(request.target_path, false, "Leaf path");
      static_cast<void>(RequireCanonicalNode(alias_document, path,
                                             AliasDocumentNodeKind::kLeaf,
                                             "leaf"));
      auto parent_path = path;
      const std::string key = parent_path.back();
      parent_path.pop_back();
      mutable_parent(parent_path)->erase(key);
      break;
    }
    case config::ActivityHierarchyOperationKind::kPromoteLeaf: {
      const auto path = ResolveLeafPath(alias_document, request);
      const auto source = RequireCanonicalNode(alias_document, path,
                                               AliasDocumentNodeKind::kLeaf,
                                               "leaf");
      auto parent_path = path;
      const std::string key = parent_path.back();
      parent_path.pop_back();
      toml::table* parent = mutable_parent(parent_path);
      toml::table group;
      toml::array group_aliases;
      for (const auto& alias : source.node->aliases) {
        group_aliases.push_back(alias.value);
      }
      group.insert("group_aliases", std::move(group_aliases));
      parent->erase(key);
      parent->insert(key, std::move(group));
      break;
    }
    case config::ActivityHierarchyOperationKind::kMoveLeaf: {
      const auto source_path = ResolveLeafPath(alias_document, request);
      const auto destination_path = ParseHierarchyPath(
          request.destination_path, false, "Destination group path");
      const auto source = RequireCanonicalNode(alias_document, source_path,
                                               AliasDocumentNodeKind::kLeaf,
                                               "leaf");
      static_cast<void>(RequireCanonicalNode(alias_document, destination_path,
                                             AliasDocumentNodeKind::kGroup,
                                             "destination group"));
      auto source_parent_path = source_path;
      const std::string key = source_parent_path.back();
      source_parent_path.pop_back();
      if (PathsEqual(source_parent_path, destination_path)) {
        throw std::invalid_argument(
            "Canonical leaf is already in the target group.");
      }
      toml::table* destination = mutable_parent(destination_path);
      if (destination->contains(key)) {
        throw std::invalid_argument("Canonical key already exists: " + key);
      }
      toml::array values;
      for (const auto& alias : source.node->aliases) {
        values.push_back(alias.value);
      }
      mutable_parent(source_parent_path)->erase(key);
      destination->insert(key, std::move(values));
      result.replacements.push_back({
          source.canonical,
          tracer::core::infrastructure::config::loader::detail::
              BuildAliasCanonicalPath(alias_document.parent, destination_path,
                                      key),
      });
      break;
    }
    case config::ActivityHierarchyOperationKind::kSetGroupAliases: {
      const auto path =
          ParseHierarchyPath(request.target_path, false, "Group path");
      static_cast<void>(RequireCanonicalNode(alias_document, path,
                                             AliasDocumentNodeKind::kGroup,
                                             "group"));
      toml::table* group = mutable_parent(path);
      group->erase("group_aliases");
      if (!request.aliases.empty()) {
        group->insert("group_aliases", MakeAliasArray(request.aliases));
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kRenameParent: {
      if (!request.old_parent.empty() &&
          request.old_parent != alias_document.parent) {
        throw std::invalid_argument(
            "Old parent does not match the alias TOML parent: " +
            request.old_parent + " != " + alias_document.parent);
      }
      ValidateParentName(request.new_name);
      if (request.new_name == alias_document.parent) {
        throw std::invalid_argument(
            "New parent name must differ from the current parent.");
      }
      for (const auto& node : tracer::core::infrastructure::config::loader::
               detail::CollectAliasDocumentCanonicalNodes(alias_document)) {
        result.replacements.push_back(
            {node.canonical,
             tracer::core::infrastructure::config::loader::detail::
                 BuildAliasCanonicalPath(
                     request.new_name,
                     node.kind == AliasDocumentNodeKind::kGroup
                         ? node.path
                         : std::vector<std::string>(node.path.begin(),
                                                    node.path.end() - 1),
                     node.kind == AliasDocumentNodeKind::kGroup
                         ? std::string_view{}
                         : node.path.back())});
      }
      document.erase("parent");
      document.insert("parent", request.new_name);
      break;
    }
    case config::ActivityHierarchyOperationKind::kRenameGroupCanonical:
    case config::ActivityHierarchyOperationKind::kRenameLeafCanonical: {
      const bool group = request.kind ==
                         config::ActivityHierarchyOperationKind::
                             kRenameGroupCanonical;
      const auto path = ParseHierarchyPath(
          request.target_path, false, group ? "Group path" : "Leaf path");
      ValidateNewKey(request.new_name);
      const auto source = RequireCanonicalNode(
          alias_document, path,
          group ? AliasDocumentNodeKind::kGroup : AliasDocumentNodeKind::kLeaf,
          group ? "group" : "leaf");
      auto parent_path = path;
      parent_path.pop_back();
      const std::string new_canonical =
          tracer::core::infrastructure::config::loader::detail::
              BuildAliasCanonicalPath(alias_document.parent, parent_path,
                                      request.new_name);
      result.replacements = CollectRenamedCanonicalReplacements(
          alias_document, path, request.new_name);
      RenameOne({source.canonical, new_canonical}, *aliases, alias_document);
      if (!group && !request.aliases.empty()) {
        toml::table* parent = mutable_parent(parent_path);
        parent->erase(request.new_name);
        parent->insert(request.new_name, MakeAliasArray(request.aliases));
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kAppendLeafAlias: {
      const auto parent_path =
          ParseHierarchyPath(request.target_path, true, "Parent group path");
      ValidateNewKey(request.canonical_key);
      const auto appended = MakeAliasArray(request.aliases);
      toml::table* parent = mutable_parent(parent_path);
      if (!parent->contains(request.canonical_key)) {
        parent->insert(request.canonical_key, std::move(appended));
        break;
      }
      toml::node& leaf = parent->at(request.canonical_key);
      auto* existing = leaf.as_array();
      if (existing == nullptr) {
        throw std::invalid_argument("Canonical key is already used as a group: " +
                                    request.canonical_key);
      }
      for (const auto& alias : request.aliases) {
        existing->push_back(alias);
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kAppendGroupAlias: {
      const auto path =
          ParseHierarchyPath(request.target_path, false, "Group path");
      static_cast<void>(RequireCanonicalNode(alias_document, path,
                                             AliasDocumentNodeKind::kGroup,
                                             "group"));
      const auto appended = MakeAliasArray(request.aliases);
      toml::table* group = mutable_parent(path);
      auto* existing = group->get_as<toml::array>("group_aliases");
      if (existing == nullptr) {
        group->insert("group_aliases", std::move(appended));
      } else {
        for (const auto& alias : request.aliases) {
          existing->push_back(alias);
        }
      }
      break;
    }
    case config::ActivityHierarchyOperationKind::kRenameGroupAlias: {
      const auto path =
          ParseHierarchyPath(request.target_path, false, "Group path");
      if (request.old_alias.empty() || request.new_name.empty()) {
        throw std::invalid_argument(
            "Old and new group aliases must not be empty.");
      }
      static_cast<void>(RequireCanonicalNode(alias_document, path,
                                             AliasDocumentNodeKind::kGroup,
                                             "group"));
      toml::table* group = mutable_parent(path);
      auto* existing = group->get_as<toml::array>("group_aliases");
      if (existing == nullptr) {
        throw std::invalid_argument("Group alias not found: " +
                                    request.old_alias);
      }
      bool replaced = false;
      for (auto alias = existing->cbegin(); alias != existing->cend(); ++alias) {
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

  const AliasDocument updated =
      tracer::core::infrastructure::config::loader::detail::ParseAliasDocument(
          document);
  tracer::core::infrastructure::config::loader::detail::
      ValidateAliasDocumentAliasUniqueness(updated);
  result.updated_toml_content = SerializeAliasDocument(document);
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
    throw std::invalid_argument("Alias TOML document set must not be empty.");
  }
  if (source_name.empty() || destination_name.empty()) {
    throw std::invalid_argument(
        "Source and destination alias TOML names must not be empty.");
  }
  if (source_name == destination_name) {
    throw std::invalid_argument(
        "Cross-document leaf move requires different source and destination documents.");
  }

  ValidateActivityHierarchyDocuments(documents);

  std::size_t source_index = documents.size();
  std::size_t destination_index = documents.size();
  std::vector<toml::table> tables;
  tables.reserve(documents.size());
  std::vector<AliasDocument> parsed_documents;
  parsed_documents.reserve(documents.size());
  for (std::size_t index = 0U; index < documents.size(); ++index) {
    if (documents[index].source_name == source_name) {
      if (source_index != documents.size()) {
        throw std::invalid_argument("Duplicate source alias TOML name: " +
                                    std::string(source_name));
      }
      source_index = index;
    }
    if (documents[index].source_name == destination_name) {
      if (destination_index != documents.size()) {
        throw std::invalid_argument(
            "Duplicate destination alias TOML name: " +
            std::string(destination_name));
      }
      destination_index = index;
    }
    tables.push_back(toml::parse(documents[index].toml_content));
    parsed_documents.push_back(
        infrastructure::config::loader::detail::ParseAliasDocument(
            tables.back()));
  }
  if (source_index == documents.size()) {
    throw std::invalid_argument("Source alias TOML not found: " +
                                std::string(source_name));
  }
  if (destination_index == documents.size()) {
    throw std::invalid_argument("Destination alias TOML not found: " +
                                std::string(destination_name));
  }

  if (request.kind != config::ActivityHierarchyOperationKind::kMoveLeaf &&
      request.kind != config::ActivityHierarchyOperationKind::kMoveGroup) {
    throw std::invalid_argument(
        "Cross-document move supports only move_leaf and move_group.");
  }
  const auto source_path =
      request.kind == config::ActivityHierarchyOperationKind::kMoveLeaf
          ? ResolveLeafPath(parsed_documents[source_index], request)
          : ResolveGroupPath(parsed_documents[source_index], request);
  const auto source = RequireCanonicalNode(parsed_documents[source_index],
                                           source_path,
                                           request.kind ==
                                                   config::ActivityHierarchyOperationKind::kMoveLeaf
                                               ? AliasDocumentNodeKind::kLeaf
                                               : AliasDocumentNodeKind::kGroup,
                                           request.kind ==
                                                   config::ActivityHierarchyOperationKind::kMoveLeaf
                                               ? "leaf"
                                               : "group");
  auto source_parent_path = source_path;
  const std::string source_key = source_parent_path.back();
  source_parent_path.pop_back();

  std::vector<std::string> destination_path;
  if (request.destination_path == "root") {
    destination_path = {};
  } else {
    destination_path = ParseHierarchyPath(request.destination_path, false,
                                           "Destination group path");
    static_cast<void>(RequireCanonicalNode(
        parsed_documents[destination_index], destination_path,
        AliasDocumentNodeKind::kGroup, "destination group"));
  }

  toml::table* source_aliases = tables[source_index]["aliases"].as_table();
  toml::table* destination_aliases =
      tables[destination_index]["aliases"].as_table();
  if (source_aliases == nullptr || destination_aliases == nullptr) {
    throw std::invalid_argument("Alias TOML must contain an `aliases` table.");
  }
  toml::table* source_parent = FindTable(*source_aliases, source_parent_path);
  toml::table* destination_parent =
      FindTable(*destination_aliases, destination_path);
  if (source_parent == nullptr || destination_parent == nullptr) {
    throw std::invalid_argument("Alias group not found.");
  }
  if (destination_parent->contains(source_key)) {
    throw std::invalid_argument("Canonical key already exists in destination: " +
                                source_key);
  }

  toml::table moved_group;
  toml::array moved_values;
  if (request.kind == config::ActivityHierarchyOperationKind::kMoveGroup) {
    moved_group = *source_parent->at(source_key).as_table();
  } else {
    for (const auto& alias : source.node->aliases) {
      moved_values.push_back(alias.value);
    }
  }
  source_parent->erase(source_key);
  if (request.kind == config::ActivityHierarchyOperationKind::kMoveGroup) {
    destination_parent->insert(source_key, std::move(moved_group));
  } else {
    destination_parent->insert(source_key, std::move(moved_values));
  }

  std::vector<AliasCanonicalReplacement> replacements;
  for (const auto& node :
       infrastructure::config::loader::detail::CollectAliasDocumentCanonicalNodes(
           parsed_documents[source_index])) {
    if (!IsPathPrefix(source_path, node.path)) {
      continue;
    }
    auto new_path = destination_path;
    new_path.push_back(source_key);
    new_path.insert(new_path.end(), node.path.begin() + source_path.size(),
                    node.path.end());
    const auto old_canonical = BuildNodeCanonical(
        parsed_documents[source_index].parent, node.path, node.kind);
    const auto new_canonical = BuildNodeCanonical(
        parsed_documents[destination_index].parent, new_path, node.kind);
    if (old_canonical != new_canonical) {
      replacements.push_back({old_canonical, new_canonical});
    }
  }

  std::vector<ActivityHierarchyDocumentInput> updated_documents;
  updated_documents.reserve(documents.size());
  for (std::size_t index = 0U; index < documents.size(); ++index) {
    updated_documents.push_back({documents[index].source_name,
                                 SerializeAliasDocument(tables[index])});
  }
  ValidateActivityHierarchyDocuments(updated_documents);

  ActivityHierarchyCrossDocumentOperationResult result;
  result.updated_documents.push_back(
      {std::string(source_name), updated_documents[source_index].toml_content});
  result.updated_documents.push_back({std::string(destination_name),
                                      updated_documents[destination_index]
                                          .toml_content});
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

auto RewriteActivityHierarchyDocument(
    std::string_view original_toml_content,
    std::string_view updated_toml_content) -> ActivityHierarchyOperationResult {
  if (original_toml_content.empty() || updated_toml_content.empty()) {
    throw std::invalid_argument("Alias TOML content must not be empty.");
  }
  const toml::table original_table = toml::parse(original_toml_content);
  const toml::table updated_table = toml::parse(updated_toml_content);
  const AliasDocument original =
      infrastructure::config::loader::detail::ParseAliasDocument(
          original_table);
  const AliasDocument updated =
      infrastructure::config::loader::detail::ParseAliasDocument(
          updated_table);
  infrastructure::config::loader::detail::ValidateAliasDocumentAliasUniqueness(
      original);
  infrastructure::config::loader::detail::ValidateAliasDocumentAliasUniqueness(
      updated);

  ActivityHierarchyOperationResult result;
  AppendRawDocumentMigrationPlan(original, updated, {}, {}, result.replacements,
                                 result.alias_replacements);
  result.updated_toml_content = SerializeAliasDocument(updated_table);
  return result;
}

auto DescribeActivityHierarchy(std::string_view toml_content)
    -> ActivityHierarchySnapshot {
  const toml::table parsed = toml::parse(toml_content);
  const AliasDocument document =
      infrastructure::config::loader::detail::ParseAliasDocument(parsed);
  infrastructure::config::loader::detail::ValidateAliasDocumentAliasUniqueness(
      document);
  ActivityHierarchySnapshot snapshot{.parent = document.parent};
  for (const auto& node : document.nodes) {
    snapshot.nodes.push_back(DescribeNode(node, {}));
  }
  return snapshot;
}

auto ValidateActivityHierarchyDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents) -> void {
  std::map<std::string, std::string> alias_sources;
  for (const auto& input : documents) {
    if (input.source_name.empty()) {
      throw std::invalid_argument("Alias document source name must not be empty.");
    }
    const toml::table parsed = toml::parse(input.toml_content);
    const AliasDocument document =
        infrastructure::config::loader::detail::ParseAliasDocument(parsed);
    infrastructure::config::loader::detail::ValidateAliasDocumentAliasUniqueness(
        document);
    for (const auto& node :
         infrastructure::config::loader::detail::CollectAliasDocumentCanonicalNodes(
             document)) {
      for (const auto& alias : node.node->aliases) {
        const auto [existing, inserted] =
            alias_sources.emplace(alias.value, input.source_name);
        if (!inserted) {
          throw std::runtime_error(
              "Duplicate alias key `" + alias.value +
              "` across alias documents: " + existing->second + " and " +
              input.source_name + ".");
        }
      }
    }
  }
}

}  // namespace tracer::core::application::config

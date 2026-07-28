#include "api/c_api/capabilities/config/alias_hierarchy_operation_bridge.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/config/alias_toml_editor.hpp"

namespace tracer_core::shell::config_bridge {
namespace {

using nlohmann::json;
namespace config = tracer::core::application::config;

[[nodiscard]] auto ToJson(const config::AliasHierarchyNodeSnapshot& node)
    -> json {
  json children = json::array();
  for (const auto& child : node.children) {
    children.push_back(ToJson(child));
  }
  return {{"canonical_key", node.canonical_key},
          {"path", node.path},
          {"is_group", node.is_group},
          {"aliases", node.aliases},
          {"children", std::move(children)}};
}

[[nodiscard]] auto ToJson(const config::AliasHierarchySnapshot& snapshot)
    -> json {
  json nodes = json::array();
  for (const auto& node : snapshot.nodes) {
    nodes.push_back(ToJson(node));
  }
  return {{"parent", snapshot.parent}, {"nodes", std::move(nodes)}};
}

[[nodiscard]] auto RequireStringField(const json& payload,
                                      std::string_view field_name)
    -> std::string {
  const auto it = payload.find(std::string(field_name));
  if (it == payload.end() || !it->is_string()) {
    throw std::invalid_argument("field `" + std::string(field_name) +
                                "` must be a string.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto ReadOptionalStringField(const json& payload,
                                           std::string_view field_name)
    -> std::string {
  const auto it = payload.find(std::string(field_name));
  if (it == payload.end() || it->is_null()) {
    return {};
  }
  if (!it->is_string()) {
    throw std::invalid_argument("field `" + std::string(field_name) +
                                "` must be a string when present.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto ReadOptionalAliases(const json& operation)
    -> std::vector<std::string> {
  const auto it = operation.find("aliases");
  if (it == operation.end() || it->is_null()) {
    return {};
  }
  if (!it->is_array()) {
    throw std::invalid_argument("field `operation.aliases` must be an array.");
  }

  std::vector<std::string> aliases;
  aliases.reserve(it->size());
  for (const auto& alias : *it) {
    if (!alias.is_string()) {
      throw std::invalid_argument(
          "each `operation.aliases` item must be a string.");
    }
    aliases.push_back(alias.get<std::string>());
  }
  return aliases;
}

[[nodiscard]] auto ParseOperationKind(std::string_view kind)
    -> config::AliasHierarchyOperationKind {
  using Kind = config::AliasHierarchyOperationKind;
  if (kind == "add_group") {
    return Kind::kAddGroup;
  }
  if (kind == "delete_group") {
    return Kind::kDeleteGroup;
  }
  if (kind == "add_leaf") {
    return Kind::kAddLeaf;
  }
  if (kind == "set_leaf_aliases") {
    return Kind::kSetLeafAliases;
  }
  if (kind == "delete_leaf") {
    return Kind::kDeleteLeaf;
  }
  if (kind == "promote_leaf") {
    return Kind::kPromoteLeaf;
  }
  if (kind == "move_leaf") {
    return Kind::kMoveLeaf;
  }
  if (kind == "set_group_aliases") {
    return Kind::kSetGroupAliases;
  }
  if (kind == "rename_parent") {
    return Kind::kRenameParent;
  }
  if (kind == "rename_group_canonical") {
    return Kind::kRenameGroupCanonical;
  }
  if (kind == "rename_leaf_canonical") {
    return Kind::kRenameLeafCanonical;
  }
  if (kind == "append_leaf_alias") {
    return Kind::kAppendLeafAlias;
  }
  if (kind == "append_group_alias") {
    return Kind::kAppendGroupAlias;
  }
  if (kind == "rename_group_alias") {
    return Kind::kRenameGroupAlias;
  }
  throw std::invalid_argument("Unsupported alias hierarchy operation kind: " +
                              std::string(kind));
}

[[nodiscard]] auto ParseOperationRequest(const json& payload)
    -> config::AliasHierarchyOperationRequest {
  const auto operation_it = payload.find("operation");
  if (operation_it == payload.end() || !operation_it->is_object()) {
    throw std::invalid_argument("field `operation` must be an object.");
  }
  const json& operation = *operation_it;
  return {
      .kind = ParseOperationKind(RequireStringField(operation, "kind")),
      .target_path = ReadOptionalStringField(operation, "target_path"),
      .destination_path =
          ReadOptionalStringField(operation, "destination_path"),
      .canonical_key = ReadOptionalStringField(operation, "canonical_key"),
      .new_name = ReadOptionalStringField(operation, "new_name"),
      .target_alias = ReadOptionalStringField(operation, "target_alias"),
      .old_alias = ReadOptionalStringField(operation, "old_alias"),
      .aliases = ReadOptionalAliases(operation),
  };
}

[[nodiscard]] auto ToOperationResultJson(
    const config::AliasHierarchyOperationResult& result) -> json {
  const auto hierarchy = config::DescribeAliasHierarchy(result.updated_toml_content);
  json replacements = json::array();
  for (const auto& replacement : result.replacements) {
    replacements.push_back({{"old_canonical", replacement.old_canonical},
                            {"new_canonical", replacement.new_canonical}});
  }
  json alias_replacements = json::array();
  for (const auto& replacement : result.alias_replacements) {
    alias_replacements.push_back(
        {{"old_alias", replacement.old_alias},
         {"new_alias", replacement.new_alias}});
  }
  return {{"updated_toml_content", result.updated_toml_content},
          {"replacements", std::move(replacements)},
          {"alias_replacements", std::move(alias_replacements)},
          {"hierarchy", ToJson(hierarchy)}};
}

}  // namespace

auto ApplyAliasHierarchyOperationJson(const nlohmann::json& payload)
    -> nlohmann::json {
  const auto result = config::ApplyAliasHierarchyOperation(
      RequireStringField(payload, "toml_content"), ParseOperationRequest(payload));
  return ToOperationResultJson(result);
}

auto RewriteAliasHierarchyDocumentJson(const nlohmann::json& payload)
    -> nlohmann::json {
  return ToOperationResultJson(config::RewriteAliasHierarchyDocument(
      RequireStringField(payload, "original_toml_content"),
      RequireStringField(payload, "updated_toml_content")));
}

auto DescribeAliasHierarchyJson(const nlohmann::json& payload)
    -> nlohmann::json {
  return {{"hierarchy",
           ToJson(config::DescribeAliasHierarchy(
               RequireStringField(payload, "toml_content")))}};
}

auto ValidateAliasHierarchyDocumentsJson(const nlohmann::json& payload)
    -> nlohmann::json {
  const auto documents_it = payload.find("documents");
  if (documents_it == payload.end() || !documents_it->is_array()) {
    throw std::invalid_argument("field `documents` must be an array.");
  }
  std::vector<config::AliasHierarchyDocumentInput> documents;
  documents.reserve(documents_it->size());
  for (const auto& item : *documents_it) {
    if (!item.is_object()) {
      throw std::invalid_argument("each `documents` item must be an object.");
    }
    documents.push_back({
        .source_name = RequireStringField(item, "source_name"),
        .toml_content = RequireStringField(item, "toml_content"),
    });
  }
  config::ValidateAliasHierarchyDocuments(documents);
  return json::object();
}

auto CreateAliasHierarchyDocumentJson(const nlohmann::json& payload)
    -> nlohmann::json {
  return {{"toml_content", config::CreateAliasHierarchyDocument(
                               RequireStringField(payload, "parent"))}};
}

}  // namespace tracer_core::shell::config_bridge

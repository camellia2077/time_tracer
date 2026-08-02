#include "api/c_api/capabilities/config/activity_hierarchy_operation_bridge.hpp"

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

[[nodiscard]] auto ToString(config::ActivityHierarchyNodeKind kind)
    -> std::string_view {
  return kind == config::ActivityHierarchyNodeKind::kGroup ? "group" : "leaf";
}

[[nodiscard]] auto ToJson(const config::ActivityHierarchyNodeSnapshot& node)
    -> json {
  json children = json::array();
  for (const auto& child : node.children) {
    children.push_back(ToJson(child));
  }
  return {{"canonical_key", node.canonical_key},
          {"path", node.path},
          {"kind", ToString(node.kind)},
          // Kept for older clients. New clients should use `kind`.
          {"is_group", node.IsGroup()},
           {"aliases", node.aliases},
          {"children", std::move(children)}};
}

[[nodiscard]] auto ToJson(const config::ActivityHierarchySnapshot& snapshot)
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
    -> config::ActivityHierarchyOperationKind {
  using Kind = config::ActivityHierarchyOperationKind;
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
  if (kind == "move_group") {
    return Kind::kMoveGroup;
  }
  if (kind == "merge_leaf_canonical") {
    return Kind::kMergeLeafCanonical;
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
  throw std::invalid_argument("Unsupported activity hierarchy operation kind: " +
                              std::string(kind));
}

[[nodiscard]] auto ParseOperationRequest(const json& payload)
    -> config::ActivityHierarchyOperationRequest {
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
      .old_parent = ReadOptionalStringField(operation, "old_parent"),
      .target_alias = ReadOptionalStringField(operation, "target_alias"),
      .old_alias = ReadOptionalStringField(operation, "old_alias"),
      .aliases = ReadOptionalAliases(operation),
  };
}

[[nodiscard]] auto ToOperationResultJson(
    const config::ActivityHierarchyOperationResult& result) -> json {
  const auto hierarchy = config::DescribeActivityHierarchy(result.updated_toml_content);
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

[[nodiscard]] auto ToCrossDocumentOperationResultJson(
    const config::ActivityHierarchyCrossDocumentOperationResult& result) -> json {
  json updated_documents = json::array();
  for (const auto& document : result.updated_documents) {
    updated_documents.push_back({{"source_name", document.source_name},
                                 {"updated_toml_content",
                                  document.updated_toml_content}});
  }
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
  return {{"updated_documents", std::move(updated_documents)},
          {"replacements", std::move(replacements)},
          {"alias_replacements", std::move(alias_replacements)}};
}

}  // namespace

auto ApplyActivityHierarchyOperationJson(const nlohmann::json& payload)
    -> nlohmann::json {
  const auto result = config::ApplyActivityHierarchyOperation(
      RequireStringField(payload, "toml_content"), ParseOperationRequest(payload));
  return ToOperationResultJson(result);
}

auto MoveActivityHierarchyLeafBetweenDocumentsJson(const nlohmann::json& payload)
    -> nlohmann::json {
  return MoveActivityHierarchyNodeBetweenDocumentsJson(payload);
}

auto MoveActivityHierarchyNodeBetweenDocumentsJson(const nlohmann::json& payload)
    -> nlohmann::json {
  const auto documents_it = payload.find("documents");
  if (documents_it == payload.end() || !documents_it->is_array()) {
    throw std::invalid_argument("field `documents` must be an array.");
  }
  std::vector<config::ActivityHierarchyDocumentInput> documents;
  documents.reserve(documents_it->size());
  for (const auto& item : *documents_it) {
    if (!item.is_object()) {
      throw std::invalid_argument("each `documents` item must be an object.");
    }
    documents.push_back({RequireStringField(item, "source_name"),
                         RequireStringField(item, "toml_content")});
  }
  return ToCrossDocumentOperationResultJson(
      config::MoveActivityHierarchyNodeBetweenDocuments(
          documents, RequireStringField(payload, "source_name"),
          RequireStringField(payload, "destination_name"),
          ParseOperationRequest(payload)));
}

auto RewriteActivityHierarchyDocumentJson(const nlohmann::json& payload)
    -> nlohmann::json {
  return ToOperationResultJson(config::RewriteActivityHierarchyDocument(
      RequireStringField(payload, "original_toml_content"),
      RequireStringField(payload, "updated_toml_content")));
}

auto DescribeActivityHierarchyJson(const nlohmann::json& payload)
    -> nlohmann::json {
  return {{"hierarchy",
           ToJson(config::DescribeActivityHierarchy(
               RequireStringField(payload, "toml_content")))}};
}

auto ValidateActivityHierarchyDocumentsJson(const nlohmann::json& payload)
    -> nlohmann::json {
  const auto documents_it = payload.find("documents");
  if (documents_it == payload.end() || !documents_it->is_array()) {
    throw std::invalid_argument("field `documents` must be an array.");
  }
  std::vector<config::ActivityHierarchyDocumentInput> documents;
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
  config::ValidateActivityHierarchyDocuments(documents);
  return json::object();
}

}  // namespace tracer_core::shell::config_bridge

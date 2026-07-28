#ifndef TRACER_CORE_APPLICATION_PORTS_CONFIG_ALIAS_TOML_EDITOR_HPP_
#define TRACER_CORE_APPLICATION_PORTS_CONFIG_ALIAS_TOML_EDITOR_HPP_

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace tracer::core::application::config {

struct AliasCanonicalReplacement {
  std::string old_canonical;
  std::string new_canonical;
};

struct AliasKeyReplacement {
  std::string old_alias;
  std::string new_alias;
};

enum class AliasHierarchyOperationKind {
  kAddGroup,
  kDeleteGroup,
  kAddLeaf,
  kSetLeafAliases,
  kDeleteLeaf,
  kPromoteLeaf,
  kMoveLeaf,
  kSetGroupAliases,
  kRenameParent,
  kRenameGroupCanonical,
  kRenameLeafCanonical,
  kAppendLeafAlias,
  kAppendGroupAlias,
  kRenameGroupAlias,
};

// Paths are relative to [aliases] and use dot-separated canonical keys.
// A root-level parent path is represented by `root` for add-group/add-leaf.
// `target_path` identifies the edited group or leaf; `destination_path` is
// used only by kMoveLeaf. `canonical_key` is used by add operations and
// `new_name` by rename operations.
struct AliasHierarchyOperationRequest {
  AliasHierarchyOperationKind kind = AliasHierarchyOperationKind::kAddGroup;
  std::string target_path;
  std::string destination_path;
  std::string canonical_key;
  std::string new_name;
  std::string target_alias;
  std::string old_alias;
  std::vector<std::string> aliases;
};

struct AliasHierarchyOperationResult {
  std::string updated_toml_content;
  std::vector<AliasCanonicalReplacement> replacements;
  std::vector<AliasKeyReplacement> alias_replacements;
};

struct AliasHierarchyNodeSnapshot {
  std::string canonical_key;
  std::string path;
  bool is_group = false;
  std::vector<std::string> aliases;
  std::vector<AliasHierarchyNodeSnapshot> children;
};

struct AliasHierarchySnapshot {
  std::string parent;
  std::vector<AliasHierarchyNodeSnapshot> nodes;
};

// One in-memory alias TOML document participating in a cross-file validation.
// `source_name` is diagnostic-only and is never read from the filesystem.
struct AliasHierarchyDocumentInput {
  std::string source_name;
  std::string toml_content;
};

// Applies one hierarchy edit in memory. It never reads or writes files.
// Callers own source persistence and, when replacements are present, TXT and
// database migration.
auto ApplyAliasHierarchyOperation(
    std::string_view toml_content,
    const AliasHierarchyOperationRequest& request) -> AliasHierarchyOperationResult;

// Validates a raw TOML edit and derives the canonical/alias token migration
// plan by comparing the Core-owned hierarchy before and after the edit.
auto RewriteAliasHierarchyDocument(
    std::string_view original_toml_content,
    std::string_view updated_toml_content) -> AliasHierarchyOperationResult;

// Returns the core-validated hierarchy view for presentation. Node paths are
// dot-separated canonical keys relative to [aliases].
auto DescribeAliasHierarchy(std::string_view toml_content)
    -> AliasHierarchySnapshot;

// Validates each supplied alias document and rejects alias keys duplicated
// across the document set. This is in-memory only; callers retain ownership of
// file reads and persistence.
auto ValidateAliasHierarchyDocuments(
    const std::vector<AliasHierarchyDocumentInput>& documents) -> void;

// Creates the minimal canonical alias TOML document for a new child file.
auto CreateAliasHierarchyDocument(std::string_view parent) -> std::string;

}  // namespace tracer::core::application::config

#endif  // TRACER_CORE_APPLICATION_PORTS_CONFIG_ALIAS_TOML_EDITOR_HPP_

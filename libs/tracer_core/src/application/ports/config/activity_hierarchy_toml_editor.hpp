#ifndef TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TOML_EDITOR_HPP_
#define TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TOML_EDITOR_HPP_

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/config/activity_hierarchy_tree.hpp"

namespace tracer::core::application::config {

struct AliasCanonicalReplacement {
  std::string old_canonical;
  std::string new_canonical;
};

struct AliasKeyReplacement {
  std::string old_alias;
  std::string new_alias;
};

enum class ActivityHierarchyOperationKind {
  kAddGroup,
  kDeleteGroup,
  kAddLeaf,
  kSetLeafAliases,
  kDeleteLeaf,
  kPromoteLeaf,
  kMoveLeaf,
  kMoveGroup,
  kMergeLeafCanonical,
  kSetGroupAliases,
  kRenameParent,
  kRenameGroupCanonical,
  kRenameLeafCanonical,
  kAppendLeafAlias,
  kAppendGroupAlias,
  kRenameGroupAlias,
};

// Paths are relative to [canonical] and use dot-separated canonical keys.
// A root-level parent path is represented by `root` for add-group/add-leaf.
// `target_path` identifies the edited group or leaf; `destination_path` is
// used by kMoveLeaf/kMoveGroup/kMergeLeafCanonical. `canonical_key` is used by
// add operations, `new_name` by rename operations, and `old_parent` optionally
// guards kRenameParent against a stale TOML document.
struct ActivityHierarchyOperationRequest {
  ActivityHierarchyOperationKind kind =
      ActivityHierarchyOperationKind::kAddGroup;
  std::string target_path;
  std::string destination_path;
  std::string canonical_key;
  std::string new_name;
  std::string old_parent;
  std::string target_alias;
  std::string old_alias;
  std::vector<std::string> aliases;
};

struct ActivityHierarchyOperationResult {
  std::string updated_toml_content;
  std::vector<AliasCanonicalReplacement> replacements;
  std::vector<AliasKeyReplacement> alias_replacements;
};

struct ActivityHierarchyDocumentResult {
  std::string source_name;
  std::string updated_toml_content;
};

struct ActivityHierarchyCrossDocumentOperationResult {
  std::vector<ActivityHierarchyDocumentResult> updated_documents;
  std::vector<AliasCanonicalReplacement> replacements;
  std::vector<AliasKeyReplacement> alias_replacements;
};

struct ActivityHierarchyDocumentInput;

// One in-memory canonical TOML document participating in a cross-file
// validation. `source_name` is diagnostic-only and is never read from the
// filesystem.
struct ActivityHierarchyDocumentInput {
  std::string source_name;
  std::string toml_content;
};

// Applies one hierarchy edit in memory. It never reads or writes files.
// Callers own source persistence and, when replacements are present, TXT and
// database migration.
auto ApplyActivityHierarchyOperation(
    std::string_view toml_content,
    const ActivityHierarchyOperationRequest& request)
    -> ActivityHierarchyOperationResult;

// Moves one leaf or one complete group subtree from one existing canonical TOML
// document to another. The complete document set is supplied so Core can
// validate global alias uniqueness after the move. The operation kind selects
// `kMoveLeaf` or `kMoveGroup`.
auto MoveActivityHierarchyNodeBetweenDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents,
    std::string_view source_name, std::string_view destination_name,
    const ActivityHierarchyOperationRequest& request)
    -> ActivityHierarchyCrossDocumentOperationResult;

// Compatibility wrapper for callers that only expose leaf moves.
auto MoveActivityHierarchyLeafBetweenDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents,
    std::string_view source_name, std::string_view destination_name,
    const ActivityHierarchyOperationRequest& request)
    -> ActivityHierarchyCrossDocumentOperationResult;

// Validates a raw TOML edit and derives the canonical/alias token migration
// plan by comparing the Core-owned hierarchy before and after the edit.
auto RewriteActivityHierarchyDocument(std::string_view original_toml_content,
                                      std::string_view updated_toml_content)
    -> ActivityHierarchyOperationResult;

// Returns the core-validated hierarchy view for presentation. Node paths are
// dot-separated canonical keys relative to [canonical].
auto DescribeActivityHierarchy(std::string_view toml_content)
    -> ActivityHierarchySnapshot;

// Validates each supplied canonical document and rejects alias keys duplicated
// across the document set. This is in-memory only; callers retain ownership of
// file reads and persistence.
auto ValidateActivityHierarchyDocuments(
    const std::vector<ActivityHierarchyDocumentInput>& documents) -> void;

}  // namespace tracer::core::application::config

#endif  // TRACER_CORE_APPLICATION_PORTS_CONFIG_ACTIVITY_HIERARCHY_TOML_EDITOR_HPP_

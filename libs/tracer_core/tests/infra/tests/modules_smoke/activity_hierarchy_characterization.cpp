import tracer.core.infrastructure.config;

#include "application/ports/config/alias_toml_editor.hpp"
#include "application/ports/config/activity_hierarchy_text_renderer.hpp"
#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/tests/modules_smoke/config.hpp"
#include "infra/tests/modules_smoke/support.hpp"

#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

namespace fs = std::filesystem;

constexpr std::string_view kAliasToml =
    "parent = \"exercise\"\n\n"
    "[canonical]\n"
    "walk = [\"步行\"]\n\n"
    "[canonical.cardio]\n"
    "group_aliases = [\"有氧\", \"cardio\"]\n\n"
    "[canonical.cardio.running]\n"
    "group_aliases = [\"跑步\"]\n"
    "treadmill = [\"跑步机\", \"treadmill\"]\n";

auto Check(bool condition, std::string_view message, int& failures) -> void {
  if (!condition) {
    ++failures;
    std::cerr << "[FAIL] " << message << '\n';
  }
}

auto CheckReplacements(
    const std::vector<
        tracer::core::application::config::AliasCanonicalReplacement>& actual,
    const std::vector<
        tracer::core::application::config::AliasCanonicalReplacement>& expected,
    std::string_view message, int& failures) -> void {
  const bool matches =
      actual.size() == expected.size() &&
      std::equal(actual.begin(), actual.end(), expected.begin(),
                 [](const auto& left, const auto& right) {
                   return left.old_canonical == right.old_canonical &&
                          left.new_canonical == right.new_canonical;
                 });
  Check(matches, message, failures);
}

auto ContainsReplacement(
    const std::vector<
        tracer::core::application::config::AliasCanonicalReplacement>&
        replacements,
    std::string_view old_canonical, std::string_view new_canonical) -> bool {
  return std::ranges::any_of(
      replacements, [old_canonical, new_canonical](const auto& replacement) {
        return replacement.old_canonical == old_canonical &&
               replacement.new_canonical == new_canonical;
      });
}

auto ExpectMapping(const tracer::core::infrastructure::config::loader::detail::
                       AliasMappingDefinition& definition,
                   int& failures) -> void {
  std::map<std::string, std::string> mappings;
  for (const auto& entry : definition.expanded_entries) {
    mappings.emplace(entry.alias_key, entry.canonical_value);
  }
  const std::map<std::string, std::string> expected = {
      {"cardio", "exercise_cardio"},
      {"treadmill", "exercise_cardio_running_treadmill"},
      {"步行", "exercise_walk"},
      {"有氧", "exercise_cardio"},
      {"跑步", "exercise_cardio_running"},
      {"跑步机", "exercise_cardio_running_treadmill"},
  };
  Check(mappings == expected,
        "Alias mapping expansion must preserve canonical hierarchy.", failures);
}

auto ExpectTree(const fs::path& path, int& failures) -> void {
  const std::string basic =
      tracer::core::application::config::RenderActivityHierarchyText(path,
                                                                     false);
  Check(basic ==
            "exercise\n"
            "├── cardio\n"
            "│   └── running\n"
            "│       └── treadmill\n"
            "└── walk\n",
        "Basic alias tree rendering changed.", failures);

  const std::string with_aliases =
      tracer::core::application::config::RenderActivityHierarchyText(path,
                                                                     true);
  Check(with_aliases ==
            "exercise\n"
            "├── cardio — group_aliases: cardio, 有氧\n"
            "│   └── running — group_aliases: 跑步\n"
            "│       └── treadmill — aliases: treadmill, 跑步机\n"
            "└── walk — aliases: 步行\n",
        "Alias tree rendering with aliases changed.", failures);
}

auto RenderOperationResult(const fs::path& root, std::string_view name,
                           std::string_view content) -> std::string {
  const fs::path path = root / (std::string(name) + ".toml");
  WriteSmokeFile(path, content);
  return tracer::core::application::config::RenderActivityHierarchyText(path,
                                                                        true);
}

auto ExpectHierarchyOperations(const fs::path& root, int& failures) -> void {
  namespace config = tracer::core::application::config;

  const auto snapshot = config::DescribeActivityHierarchy(kAliasToml);
  const auto walk = std::ranges::find_if(snapshot.nodes, [](const auto& node) {
    return node.canonical_key == "walk";
  });
  const auto cardio = std::ranges::find_if(
      snapshot.nodes,
      [](const auto& node) { return node.canonical_key == "cardio"; });
  Check(snapshot.parent == "exercise" && snapshot.nodes.size() == 2U &&
            walk != snapshot.nodes.end() &&
            walk->kind == config::ActivityHierarchyNodeKind::kLeaf &&
            walk->aliases == std::vector<std::string>{"步行"} &&
            cardio != snapshot.nodes.end() &&
            cardio->kind == config::ActivityHierarchyNodeKind::kGroup &&
            cardio->path == "cardio" && !cardio->children.empty() &&
            cardio->children[0].path == "cardio.running",
        "Core hierarchy snapshot changed.", failures);

  const auto add_group = config::ApplyActivityHierarchyOperation(
      kAliasToml, {
                      .kind = config::ActivityHierarchyOperationKind::kAddGroup,
                      .target_path = "root",
                      .canonical_key = "strength",
                  });
  Check(RenderOperationResult(root, "add-group", add_group.updated_toml_content)
                .find("strength\n") != std::string::npos,
        "Add group operation changed.", failures);

  const auto add_leaf = config::ApplyActivityHierarchyOperation(
      add_group.updated_toml_content,
      {
          .kind = config::ActivityHierarchyOperationKind::kAddLeaf,
          .target_path = "strength",
          .canonical_key = "squat",
          .aliases = {"深蹲"},
      });
  Check(RenderOperationResult(root, "add-leaf", add_leaf.updated_toml_content)
                .find("└── squat — aliases: 深蹲\n") != std::string::npos,
        "Add leaf operation changed.", failures);

  const auto set_group_aliases = config::ApplyActivityHierarchyOperation(
      add_leaf.updated_toml_content,
      {
          .kind = config::ActivityHierarchyOperationKind::kSetGroupAliases,
          .target_path = "strength",
          .aliases = {"力量训练"},
      });
  Check(RenderOperationResult(root, "set-group-aliases",
                              set_group_aliases.updated_toml_content)
                .find("strength — group_aliases: 力量训练\n") !=
            std::string::npos,
        "Set group aliases operation changed.", failures);

  const auto set_group_aliases_with_rename =
      config::ApplyActivityHierarchyOperation(
          kAliasToml,
          {
              .kind = config::ActivityHierarchyOperationKind::kSetGroupAliases,
              .target_path = "cardio",
              .aliases = {"有氧训练", "cardio"},
          });
  Check(set_group_aliases_with_rename.replacements.empty() &&
            set_group_aliases_with_rename.alias_replacements.size() == 1U &&
            set_group_aliases_with_rename.alias_replacements[0].old_alias ==
                "有氧" &&
            set_group_aliases_with_rename.alias_replacements[0].new_alias ==
                "有氧训练",
        "Set group aliases must return alias-key replacements for renames.",
        failures);

  const auto rename_group_alias = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kRenameGroupAlias,
          .target_path = "cardio",
          .old_alias = "有氧",
          .new_name = "有氧训练",
      });
  Check(rename_group_alias.replacements.empty() &&
            rename_group_alias.alias_replacements.size() == 1U &&
            rename_group_alias.alias_replacements[0].old_alias == "有氧" &&
            rename_group_alias.alias_replacements[0].new_alias == "有氧训练" &&
            RenderOperationResult(root, "rename-group-alias",
                                  rename_group_alias.updated_toml_content)
                    .find("cardio — group_aliases: cardio, 有氧训练\n") !=
                std::string::npos,
        "Rename group alias must return an alias-key replacement, not a "
        "canonical replacement.",
        failures);

  const auto delete_group = config::ApplyActivityHierarchyOperation(
      set_group_aliases.updated_toml_content,
      {
          .kind = config::ActivityHierarchyOperationKind::kDeleteGroup,
          .target_path = "strength",
      });
  Check(RenderOperationResult(root, "delete-group",
                              delete_group.updated_toml_content)
                .find("strength") == std::string::npos,
        "Delete group operation changed.", failures);

  const auto set_leaf_aliases = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kSetLeafAliases,
          .target_path = "walk",
          .aliases = {"散步"},
      });
  Check(RenderOperationResult(root, "set-leaf-aliases",
                              set_leaf_aliases.updated_toml_content)
                .find("walk — aliases: 散步\n") != std::string::npos,
        "Set leaf aliases operation changed.", failures);
  Check(set_leaf_aliases.alias_replacements.size() == 1U &&
            set_leaf_aliases.alias_replacements[0].old_alias == "步行" &&
            set_leaf_aliases.alias_replacements[0].new_alias == "散步",
        "Set leaf aliases must return the core alias-key migration plan.",
        failures);

  const auto append_leaf_alias = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kAppendLeafAlias,
          .target_path = "root",
          .canonical_key = "walk",
          .aliases = {"散步"},
      });
  Check(RenderOperationResult(root, "append-leaf-alias",
                              append_leaf_alias.updated_toml_content)
                .find("散步") != std::string::npos,
        "Append leaf alias operation changed.", failures);

  const auto promote = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kPromoteLeaf,
          .target_path = "walk",
      });
  Check(promote.replacements.empty() &&
            RenderOperationResult(root, "promote", promote.updated_toml_content)
                    .find("walk — group_aliases: 步行\n") != std::string::npos,
        "Promote leaf operation changed.", failures);

  const auto promote_by_alias = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kPromoteLeaf,
          .target_alias = "步行",
      });
  Check(RenderOperationResult(root, "promote-by-alias",
                              promote_by_alias.updated_toml_content)
                .find("walk — group_aliases: 步行\n") != std::string::npos,
        "Alias-selected promote operation changed.", failures);

  const auto move = config::ApplyActivityHierarchyOperation(
      kAliasToml, {
                      .kind = config::ActivityHierarchyOperationKind::kMoveLeaf,
                      .target_path = "cardio.running.treadmill",
                      .destination_path = "cardio",
                  });
  CheckReplacements(
      move.replacements,
      {{"exercise_cardio_running_treadmill", "exercise_cardio_treadmill"}},
      "Move leaf replacement plan changed.", failures);
  Check(RenderOperationResult(root, "move", move.updated_toml_content)
                .find("treadmill — aliases: treadmill, 跑步机\n") !=
            std::string::npos,
        "Move leaf operation changed.", failures);

  const auto merge = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kMergeLeafCanonical,
          .target_path = "cardio.running.treadmill",
          .destination_path = "walk",
      });
  CheckReplacements(merge.replacements,
                    {{"exercise_cardio_running_treadmill", "exercise_walk"}},
                    "Merge leaf replacement plan changed.", failures);
  Check(
      merge.alias_replacements.size() == 2U &&
          merge.alias_replacements[0].old_alias == "跑步机" &&
          merge.alias_replacements[0].new_alias == "步行" &&
          merge.alias_replacements[1].old_alias == "treadmill" &&
          merge.alias_replacements[1].new_alias == "步行" &&
          RenderOperationResult(root, "merge-leaf", merge.updated_toml_content)
                  .find("treadmill") == std::string::npos &&
          RenderOperationResult(root, "merge-leaf", merge.updated_toml_content)
                  .find("walk — aliases: 步行") != std::string::npos,
      "Merge leaf must remove the source canonical and aliases.", failures);

  bool rejected_group_merge = false;
  try {
    static_cast<void>(config::ApplyActivityHierarchyOperation(
        kAliasToml,
        {.kind = config::ActivityHierarchyOperationKind::kMergeLeafCanonical,
         .target_path = "cardio",
         .destination_path = "walk"}));
  } catch (const std::invalid_argument&) {
    rejected_group_merge = true;
  }
  Check(rejected_group_merge, "Merge leaf must reject group sources.",
        failures);

  const auto rename_parent = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kRenameParent,
          .new_name = "training",
          .old_parent = "exercise",
      });
  const auto renamed_snapshot =
      config::DescribeActivityHierarchy(rename_parent.updated_toml_content);
  Check(renamed_snapshot.parent == "training" &&
            rename_parent.replacements.size() == 4U &&
            ContainsReplacement(rename_parent.replacements, "exercise_walk",
                                "training_walk") &&
            ContainsReplacement(rename_parent.replacements, "exercise_cardio",
                                "training_cardio") &&
            ContainsReplacement(rename_parent.replacements,
                                "exercise_cardio_running",
                                "training_cardio_running") &&
            ContainsReplacement(rename_parent.replacements,
                                "exercise_cardio_running_treadmill",
                                "training_cardio_running_treadmill"),
        "Rename parent replacement plan changed.", failures);

  bool rejected_stale_parent = false;
  try {
    static_cast<void>(config::ApplyActivityHierarchyOperation(
        kAliasToml,
        {.kind = config::ActivityHierarchyOperationKind::kRenameParent,
         .new_name = "training",
         .old_parent = "stale"}));
  } catch (const std::invalid_argument&) {
    rejected_stale_parent = true;
  }
  Check(rejected_stale_parent,
        "Rename parent must reject a stale old_parent guard.", failures);

  bool rejected_same_parent = false;
  try {
    static_cast<void>(config::ApplyActivityHierarchyOperation(
        kAliasToml,
        {.kind = config::ActivityHierarchyOperationKind::kRenameParent,
         .new_name = "exercise"}));
  } catch (const std::invalid_argument&) {
    rejected_same_parent = true;
  }
  Check(rejected_same_parent, "Rename parent must reject a no-op parent name.",
        failures);

  bool rejected_unsafe_parent = false;
  try {
    static_cast<void>(config::ApplyActivityHierarchyOperation(
        kAliasToml,
        {.kind = config::ActivityHierarchyOperationKind::kRenameParent,
         .new_name = "training/2026"}));
  } catch (const std::invalid_argument&) {
    rejected_unsafe_parent = true;
  }
  Check(rejected_unsafe_parent,
        "Rename parent must reject a path-shaped parent name.", failures);

  const auto rename_group = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kRenameGroupCanonical,
          .target_path = "cardio",
          .new_name = "conditioning",
      });
  CheckReplacements(
      rename_group.replacements,
      {{"exercise_cardio", "exercise_conditioning"},
       {"exercise_cardio_running", "exercise_conditioning_running"},
       {"exercise_cardio_running_treadmill",
        "exercise_conditioning_running_treadmill"}},
      "Generic group rename replacement plan changed.", failures);

  const auto rename_leaf = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kRenameLeafCanonical,
          .target_path = "walk",
          .new_name = "stroll",
      });
  CheckReplacements(rename_leaf.replacements,
                    {{"exercise_walk", "exercise_stroll"}},
                    "Generic leaf rename replacement plan changed.", failures);

  const auto rename_leaf_with_aliases = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kRenameLeafCanonical,
          .target_path = "walk",
          .new_name = "stroll",
          .aliases = {"散步", "漫步"},
      });
  Check(RenderOperationResult(root, "rename-leaf-with-aliases",
                              rename_leaf_with_aliases.updated_toml_content)
                .find("stroll — aliases: 散步, 漫步\n") != std::string::npos,
        "Leaf rename must update aliases in the same core operation.",
        failures);

  const auto delete_leaf = config::ApplyActivityHierarchyOperation(
      kAliasToml,
      {
          .kind = config::ActivityHierarchyOperationKind::kDeleteLeaf,
          .target_path = "walk",
      });
  Check(RenderOperationResult(root, "delete-leaf",
                              delete_leaf.updated_toml_content)
                .find("walk") == std::string::npos,
        "Delete leaf operation changed.", failures);

  bool rejected_duplicate = false;
  try {
    static_cast<void>(config::ApplyActivityHierarchyOperation(
        kAliasToml,
        {
            .kind = config::ActivityHierarchyOperationKind::kAddLeaf,
            .target_path = "cardio",
            .canonical_key = "duplicate",
            .aliases = {"有氧"},
        }));
  } catch (const std::runtime_error&) {
    rejected_duplicate = true;
  }
  Check(rejected_duplicate,
        "Hierarchy operations must reject duplicate aliases globally.",
        failures);

  bool rejected_cross_file_duplicate = false;
  try {
    config::ValidateActivityHierarchyDocuments(
        std::vector<config::ActivityHierarchyDocumentInput>{
            {"one.toml", std::string(kAliasToml)},
            {"two.toml",
             "parent = \"rest\"\n\n[canonical]\nrest = [\"步行\"]\n"}});
  } catch (const std::runtime_error&) {
    rejected_cross_file_duplicate = true;
  }
  Check(
      rejected_cross_file_duplicate,
      "Core document-set validation must reject cross-file duplicate aliases.",
      failures);
}

auto ExpectDuplicateAliasRejection(const fs::path& directory, int& failures)
    -> void {
  WriteSmokeFile(directory / "duplicate.toml",
                 "parent = \"duplicate\"\n\n[canonical]\n"
                 "one = [\"same\"]\n"
                 "two = [\"same\"]\n");
  bool rejected = false;
  try {
    static_cast<void>(
        tracer::core::infrastructure::config::loader::detail::
            LoadAliasMappingDefinition(
                directory,
                tracer::core::infrastructure::config::loader::ReadToml));
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("Duplicate alias key `same`") !=
               std::string::npos;
  }
  Check(rejected, "Duplicate aliases must be rejected globally.", failures);
}

auto ExpectEmptyAliasDirectoryIsValid(const fs::path& directory, int& failures)
    -> void {
  std::error_code cleanup_error;
  fs::remove_all(directory, cleanup_error);
  fs::create_directories(directory);

  const auto definition = tracer::core::infrastructure::config::loader::detail::
      LoadAliasMappingDefinition(
          directory, tracer::core::infrastructure::config::loader::ReadToml);
  Check(definition.child_files.empty(),
        "An empty activity hierarchy directory must have no child files.",
        failures);
  Check(definition.expanded_entries.empty(),
        "An empty activity hierarchy directory must have no mappings.",
        failures);
}

}  // namespace

auto RunActivityHierarchyCharacterizationTests() -> int {
  int failures = 0;
  std::error_code cleanup_error;
  const fs::path root =
      fs::path("temp") / "activity_hierarchy_characterization";
  fs::remove_all(root, cleanup_error);

  const fs::path alias_directory = root / "activity_hierarchy";
  const fs::path alias_file = alias_directory / "exercise.toml";
  WriteSmokeFile(alias_file, kAliasToml);

  const auto definition = tracer::core::infrastructure::config::loader::detail::
      LoadAliasMappingDefinition(
          alias_directory,
          tracer::core::infrastructure::config::loader::ReadToml);
  ExpectMapping(definition, failures);
  ExpectTree(alias_file, failures);
  ExpectHierarchyOperations(root, failures);
  ExpectDuplicateAliasRejection(root / "duplicates", failures);
  ExpectEmptyAliasDirectoryIsValid(root / "empty_activity_hierarchy", failures);

  fs::remove_all(root, cleanup_error);
  return failures == 0 ? 0 : 1;
}

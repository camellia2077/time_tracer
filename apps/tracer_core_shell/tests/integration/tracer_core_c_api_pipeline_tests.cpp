#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunPipelineChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                       const fs::path& input_root) {
  RequireOk(
      api.runtime_validate_structure(
          runtime, json{{"input_path", input_root.string()}}.dump().c_str()),
      "baseline pipeline validate structure");

  RequireOk(api.runtime_validate_logic(runtime,
                                       json{{"input_path", input_root.string()},
                                            {"date_check_mode", "none"}}
                                           .dump()
                                           .c_str()),
            "baseline pipeline validate logic");

  RequireOk(
      api.runtime_convert(runtime, json{{"input_path", input_root.string()},
                                        {"date_check_mode", "none"},
                                        {"save_processed_output", false},
                                        {"validate_logic", true},
                                        {"validate_structure", true}}
                                       .dump()
                                       .c_str()),
      "baseline pipeline convert");

  RequireOk(
      api.runtime_ingest(runtime, json{{"input_path", input_root.string()},
                                       {"date_check_mode", "none"},
                                       {"save_processed_output", false}}
                                      .dump()
                                      .c_str()),
      "baseline pipeline ingest");

  const json kSyncStatus = ParseResponse(
      api.runtime_ingest_sync_status(runtime, json::object().dump().c_str()),
      "baseline pipeline ingest sync status");
  Require(kSyncStatus.value("ok", false),
          "ingest sync status should return ok=true");
  Require(kSyncStatus.contains("items") && kSyncStatus["items"].is_array(),
          "ingest sync status should include array field `items`");

  RequireOk(api.runtime_clear_ingest_sync_status(runtime),
            "baseline pipeline clear ingest sync status");

  const json kClearedSyncStatus =
      ParseResponse(api.runtime_ingest_sync_status(
                        runtime, json::object().dump().c_str()),
                    "baseline pipeline ingest sync status after clear");
  Require(kClearedSyncStatus.value("ok", false),
          "ingest sync status after clear should return ok=true");
  Require(kClearedSyncStatus["items"].empty(),
          "ingest sync status after clear should return no items");

  const json kInvalidTimeOrderMode = ParseResponse(
      api.runtime_record_activity_atomically(
          runtime, json{{"target_date_iso", "2026-03-29"},
                        {"raw_activity_name", "study"},
                        {"remark", ""},
                        {"time_order_mode", "invalid_mode"}}
                       .dump()
                       .c_str()),
      "record activity atomically invalid time_order_mode");
  Require(!kInvalidTimeOrderMode.value("ok", true),
          "invalid time_order_mode should return ok=false");
  const std::string kInvalidModeError =
      kInvalidTimeOrderMode.value("error_message", "");
  Require(kInvalidModeError.find("strict_calendar|logical_day_0600") !=
              std::string::npos,
          "invalid time_order_mode error should list allowed values");

  const json kResolveDay = ParseResponse(
      api.runtime_txt(runtime, json{{"action", "resolve_day_block"},
                                    {"content", std::string(
                                                    "y2025\nm01\n\nd0101\n0900study\n\nd0102\n0656w\n0904无氧训练\n")},
                                    {"day_marker", "0102"},
                                    {"selected_month", "2025-01"}}
                                   .dump()
                                   .c_str()),
      "txt resolve day");
  Require(kResolveDay.value("ok", false),
          "resolve_day_block should return ok=true");
  Require(kResolveDay.value("found", false),
          "resolve_day_block should find existing day marker");
  Require(kResolveDay.value("is_marker_valid", false),
          "resolve_day_block should accept valid marker");
  Require(kResolveDay.value("day_body", std::string{}).find("0904无氧训练") !=
              std::string::npos,
          "resolve_day_block should return day body without marker line");

  const json kMissingDay = ParseResponse(
      api.runtime_txt(runtime, json{{"action", "resolve_day_block"},
                                    {"content", std::string(
                                                    "y2025\nm01\n\nd0101\n0900study\n")},
                                    {"day_marker", "0102"},
                                    {"selected_month", "2025-01"}}
                                   .dump()
                                   .c_str()),
      "txt resolve missing day");
  Require(kMissingDay.value("ok", false),
          "missing day resolve should still return ok=true");
  Require(!kMissingDay.value("found", true),
          "missing day resolve should return found=false");
  Require(!kMissingDay.value("can_save", true),
          "missing day resolve should disable save");

  const json kReplaceDay = ParseResponse(
      api.runtime_txt(runtime, json{{"action", "replace_day_block"},
                                    {"content", std::string(
                                                    "y2025\nm01\n\nd0101\n0900study\n\nd0102\n0656w\n0904无氧训练\n")},
                                    {"day_marker", "0102"},
                                    {"edited_day_body", "d0102\n1111new_line\n"}}
                                   .dump()
                                   .c_str()),
      "txt replace day");
  Require(kReplaceDay.value("ok", false),
          "replace_day_block should return ok=true");
  Require(kReplaceDay.value("updated_content", std::string{}).find(
              "d0102\n1111new_line\n") != std::string::npos,
          "replace_day_block should strip duplicated marker and update content");

  const std::string kAliasMonthText =
      "y2026\nm01\n\n0830英语单词 // keep remark\n";
  const json kCanonicalActivityNames = ParseResponse(
      api.runtime_txt(runtime,
                      json{{"action", "convert_activity_names"},
                           {"content", kAliasMonthText},
                           {"direction", "alias_to_canonical"}}
                          .dump()
                          .c_str()),
      "txt convert activity names alias to canonical");
  Require(kCanonicalActivityNames.value("ok", false),
          "convert_activity_names alias_to_canonical should return ok=true");
  Require(kCanonicalActivityNames.value("converted_content", std::string{})
              .find("0830study_english_words // keep remark") !=
              std::string::npos,
          "convert_activity_names should convert aliases in month TXT");

  const json kAliasActivityNames = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "convert_activity_names"},
               {"content", kCanonicalActivityNames.value(
                               "converted_content", std::string{})},
               {"direction", "canonical_to_alias"}}
              .dump()
              .c_str()),
      "txt convert activity names canonical to alias");
  Require(kAliasActivityNames.value("ok", false),
          "convert_activity_names canonical_to_alias should return ok=true");
  Require(kAliasActivityNames.value("converted_content", std::string{})
              .find("0830英语单词 // keep remark") != std::string::npos,
          "convert_activity_names should convert canonical names to aliases");

  const json kCanonicalReplacement = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "replace_canonical_activity_names"},
               {"content", "y2026\nm01\n\n0830exercise_walk // exercise_walk remark\n"},
               {"replacements", json::array({
                   {{"old_canonical", "exercise_walk"},
                    {"new_canonical", "exercise_cardio_walk"}}})}}
              .dump()
              .c_str()),
      "txt replace canonical activity names");
  Require(kCanonicalReplacement.value("ok", false),
          "replace_canonical_activity_names should return ok=true");
  Require(kCanonicalReplacement.value("updated_content", std::string{}) ==
              "y2026\nm01\n\n0830exercise_cardio_walk // exercise_walk remark\n",
          "canonical replacement should preserve remarks and replace only event names");

  const json kAliasReplacement = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "replace_alias_activity_names"},
               {"content", "y2026\nm01\n\n012431有氧\n012500exercise_cardio // keep canonical\n"},
               {"replacements", json::array({
                   {{"old_alias", "有氧"}, {"new_alias", "有氧aa"}}})}}
              .dump()
              .c_str()),
      "txt replace alias activity names");
  Require(kAliasReplacement.value("ok", false),
          "replace_alias_activity_names should return ok=true");
  Require(kAliasReplacement.value("updated_content", std::string{}) ==
              "y2026\nm01\n\n012431有氧aa\n012500exercise_cardio // keep canonical\n",
          "alias replacement should preserve canonical names and remarks");

  const std::string kAliasToml =
      "parent = \"exercise\"\n\n"
      "[aliases.cardio]\n"
      "group_aliases = [\"有氧\"]\n\n"
      "[aliases.cardio.running]\n"
      "group_aliases = [\"跑步\"]\n"
      "treadmill = [\"跑步机\"]\n";
  const json kApplyAliasHierarchyOperation = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "apply_alias_hierarchy_operation"},
               {"toml_content", kAliasToml},
               {"operation",
                {{"kind", "rename_group_canonical"},
                 {"target_path", "cardio"},
                 {"new_name", "conditioning"}}}}
              .dump()
              .c_str()),
      "apply alias hierarchy operation");
  Require(kApplyAliasHierarchyOperation.value("ok", false),
          "apply_alias_hierarchy_operation should return ok=true");
  Require(kApplyAliasHierarchyOperation.value("updated_toml_content",
                                               std::string{})
              .find("[aliases.conditioning]") != std::string::npos,
          "generic hierarchy operation should update the group TOML key");
  Require(kApplyAliasHierarchyOperation.at("replacements").size() == 3,
          "generic group rename should return all canonical replacements");
  const auto& kHierarchy = kApplyAliasHierarchyOperation.at("hierarchy");
  Require(kHierarchy.value("parent", std::string{}) == "exercise" &&
              kHierarchy.at("nodes").at(1).value("path", std::string{}) ==
                  "conditioning",
          "generic hierarchy operation should return the core hierarchy snapshot");

  const json kDescribeAliasHierarchy = ParseResponse(
      api.runtime_txt(runtime,
                      json{{"action", "describe_alias_hierarchy"},
                           {"toml_content", kAliasToml}}
                          .dump()
                          .c_str()),
      "describe alias hierarchy");
  Require(kDescribeAliasHierarchy.value("ok", false) &&
              kDescribeAliasHierarchy.at("hierarchy")
                      .value("parent", std::string{}) == "exercise",
          "describe_alias_hierarchy should return the core hierarchy snapshot");

  const json kRenderAliasHierarchyText = ParseResponse(
      api.runtime_txt(runtime,
                      json{{"action", "render_alias_hierarchy_text"},
                           {"toml_content", kAliasToml},
                           {"show_aliases", true}}
                          .dump()
                          .c_str()),
      "render alias hierarchy text");
  Require(kRenderAliasHierarchyText.value("ok", false) &&
              kRenderAliasHierarchyText.value("content", std::string{})
                      .find("cardio — group_aliases: 有氧") !=
                  std::string::npos,
          "render_alias_hierarchy_text should render the core hierarchy");

  const json kDuplicateHierarchyDocuments = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "validate_alias_hierarchy_documents"},
               {"documents",
                {{{"source_name", "exercise.toml"},
                  {"toml_content", kAliasToml}},
                 {{"source_name", "rest.toml"},
                  {"toml_content",
                   "parent = \"rest\"\n\n[aliases]\nrest = [\"步行\"]\n"}}}}}
              .dump()
              .c_str()),
      "validate duplicate alias hierarchy documents");
  Require(!kDuplicateHierarchyDocuments.value("ok", true) &&
              kDuplicateHierarchyDocuments.value("error_code", std::string{}) ==
                  "config.alias_hierarchy.failed",
          "document-set validation should reject cross-file duplicate aliases");

  const json kMoveLeafOperation = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "apply_alias_hierarchy_operation"},
               {"toml_content", kAliasToml},
               {"operation",
                {{"kind", "move_leaf"},
                 {"target_path", "cardio.running.treadmill"},
                 {"destination_path", "cardio"}}}}
              .dump()
              .c_str()),
      "move alias hierarchy leaf");
  Require(kMoveLeafOperation.value("ok", false),
          "move_leaf hierarchy operation should return ok=true");
  const auto& kMoveReplacements = kMoveLeafOperation.at("replacements");
  Require(kMoveReplacements.size() == 1 &&
              kMoveReplacements[0].value("old_canonical", std::string{}) ==
                  "exercise_cardio_running_treadmill" &&
              kMoveReplacements[0].value("new_canonical", std::string{}) ==
                  "exercise_cardio_treadmill",
          "move_leaf should return the precise canonical replacement");

  const json kInvalidHierarchyOperation = ParseResponse(
      api.runtime_txt(
          runtime,
          json{{"action", "apply_alias_hierarchy_operation"},
               {"toml_content", kAliasToml},
               {"operation", {{"kind", "rename_everything"}}}}
              .dump()
              .c_str()),
      "invalid alias hierarchy operation");
  Require(!kInvalidHierarchyOperation.value("ok", true),
          "invalid hierarchy operation should return ok=false");
  Require(kInvalidHierarchyOperation.value("error_code", std::string{}) ==
              "config.alias_hierarchy.failed" &&
              kInvalidHierarchyOperation.value("error_category",
                                                std::string{}) == "config",
          "invalid hierarchy operation should return the config error contract");

  const json kDefaultMarker = ParseResponse(
      api.runtime_txt(runtime, json{{"action", "default_day_marker"},
                                    {"selected_month", "2025-02"},
                                    {"target_date_iso", "2025-01-31"}}
                                   .dump()
                                   .c_str()),
      "txt default day marker");
  Require(kDefaultMarker.value("ok", false),
          "default_day_marker should return ok=true");
  Require(kDefaultMarker.value("normalized_day_marker", std::string{}) ==
              "0228",
          "default_day_marker should clamp to selected month end");

  const json kInvalidTxtAction = ParseResponse(
      api.runtime_txt(runtime, json{{"action", "noop"}}.dump().c_str()),
      "txt invalid action");
  Require(!kInvalidTxtAction.value("ok", true),
          "unsupported txt action should return ok=false");
  Require(kInvalidTxtAction.value("error_code", std::string{}) ==
              "runtime.invalid_request",
          "unsupported txt action should expose runtime.invalid_request");
  Require(kInvalidTxtAction.value("error_category", std::string{}) == "runtime",
          "unsupported txt action should expose runtime category");

  const json kInvalidValidateLogic = ParseResponse(
      api.runtime_validate_logic(runtime,
                                 json{{"input_path", input_root.string()},
                                      {"date_check_mode", "invalid_mode"}}
                                     .dump()
                                     .c_str()),
      "invalid validate logic date_check_mode");
  Require(!kInvalidValidateLogic.value("ok", true),
          "invalid validate logic date_check_mode should return ok=false");
  Require(kInvalidValidateLogic.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "invalid validate logic date_check_mode should use generic runtime error code");
  Require(kInvalidValidateLogic.value("error_message", std::string{}).find(
              "field `date_check_mode` must be one of") != std::string::npos,
          "invalid validate logic date_check_mode should explain allowed values");
}

}  // namespace tracer_core_c_api_stability_internal

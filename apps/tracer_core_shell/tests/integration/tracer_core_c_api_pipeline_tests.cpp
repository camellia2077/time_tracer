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
                                                    "y2025\nm01\n\n0101\n0900study\n\n0102\n0656w\n0904无氧训练\n")},
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
                                                    "y2025\nm01\n\n0101\n0900study\n")},
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
                                                    "y2025\nm01\n\n0101\n0900study\n\n0102\n0656w\n0904无氧训练\n")},
                                    {"day_marker", "0102"},
                                    {"edited_day_body", "0102\n1111new_line\n"}}
                                   .dump()
                                   .c_str()),
      "txt replace day");
  Require(kReplaceDay.value("ok", false),
          "replace_day_block should return ok=true");
  Require(kReplaceDay.value("updated_content", std::string{}).find(
              "0102\n1111new_line\n") != std::string::npos,
          "replace_day_block should strip duplicated marker and update content");

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

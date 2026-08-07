import tracer.core.application.use_cases.interface;

#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "nlohmann/json.hpp"
#include "application/dto/pipeline_requests.hpp"
#include "application/ports/config/activity_hierarchy_toml_editor.hpp"
#include "application/ports/config/quick_access_toml_store.hpp"
#include "api/c_api/capabilities/config/activity_hierarchy_operation_bridge.hpp"
#include "application/ports/config/activity_hierarchy_text_renderer.hpp"
#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;
using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::shell::config_bridge::ApplyActivityHierarchyOperationJson;
using tracer_core::shell::config_bridge::MoveActivityHierarchyLeafBetweenDocumentsJson;
using tracer_core::shell::config_bridge::MoveActivityHierarchyNodeBetweenDocumentsJson;
using tracer_core::shell::config_bridge::RewriteActivityHierarchyDocumentJson;
using tracer_core::shell::config_bridge::DescribeActivityHierarchyJson;
using tracer_core::shell::config_bridge::ValidateActivityHierarchyDocumentsJson;

namespace {

using nlohmann::json;

template <typename Builder>
auto BuildTxtSuccessResponse(Builder&& builder) -> const char* {
  json payload = builder();
  payload["ok"] = true;
  payload["error_message"] = "";
  payload["error_code"] = "";
  payload["error_category"] = "";
  payload["hints"] = json::array();
  tracer_core::core::c_api::internal::g_last_response = payload.dump();
  return tracer_core::core::c_api::internal::g_last_response.c_str();
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
    return "";
  }
  if (!it->is_string()) {
    throw std::invalid_argument("field `" + std::string(field_name) +
                                "` must be a string when present.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto RequireStringArrayField(const json& payload,
                                           std::string_view field_name)
    -> std::vector<std::string> {
  const auto it = payload.find(std::string(field_name));
  if (it == payload.end() || !it->is_array()) {
    throw std::invalid_argument("field `" + std::string(field_name) +
                                "` must be an array.");
  }

  std::vector<std::string> values;
  values.reserve(it->size());
  for (const auto& value : *it) {
    if (!value.is_string()) {
      throw std::invalid_argument("each `" + std::string(field_name) +
                                  "` item must be a string.");
    }
    values.push_back(value.get<std::string>());
  }
  return values;
}

}  // namespace

extern "C" TT_CORE_API auto tracer_core_runtime_config_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  // This runtime family carries TXT authoring operations and the config-owned
  // activity hierarchy edit port without routing either through query/pipeline.
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const json payload = json::parse(ToRequestJsonView(request_json));
    if (!payload.is_object()) {
      throw std::invalid_argument("request_json must be a JSON object.");
    }

    const std::string action = RequireStringField(payload, "action");
    if (action == "read_quick_access") {
      try {
        const auto config = tracer::core::application::config::ParseQuickAccessToml(
            RequireStringField(payload, "toml_content"));
        return BuildTxtSuccessResponse([&]() -> json {
          return json{{"quick_access", config.aliases}};
        });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.quick_access.failed",
                                    "config", {});
      }
    }

    if (action == "write_quick_access") {
      try {
        const tracer::core::application::config::QuickAccessConfig config{
            .aliases = RequireStringArrayField(payload, "quick_access")};
        const auto toml_content =
            tracer::core::application::config::RenderQuickAccessToml(config);
        return BuildTxtSuccessResponse([&]() -> json {
          return json{{"quick_access", config.aliases},
                      {"toml_content", toml_content}};
        });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.quick_access.failed",
                                    "config", {});
      }
    }

    if (action == "default_day_marker") {
      const auto response = runtime.pipeline().RunDefaultTxtDayMarker(
          {.selected_month = ReadOptionalStringField(payload, "selected_month"),
           .target_date_iso = RequireStringField(payload, "target_date_iso")});
      if (!response.ok) {
        return BuildFailureResponse(response.error_message);
      }
      return BuildTxtSuccessResponse([&]() -> json {
        return json{{"normalized_day_marker", response.normalized_day_marker}};
      });
    }

    if (action == "resolve_day_block") {
      const auto response = runtime.pipeline().RunResolveTxtDayBlock(
          {.content = RequireStringField(payload, "content"),
           .day_marker = RequireStringField(payload, "day_marker"),
           .selected_month = ReadOptionalStringField(payload, "selected_month")});
      if (!response.ok) {
        return BuildFailureResponse(response.error_message);
      }
      return BuildTxtSuccessResponse([&]() -> json {
        json result = {{"normalized_day_marker", response.normalized_day_marker},
                       {"found", response.found},
                       {"is_marker_valid", response.is_marker_valid},
                       {"can_save", response.can_save},
                       {"day_body", response.day_body}};
        if (response.day_content_iso_date.has_value()) {
          result["day_content_iso_date"] = *response.day_content_iso_date;
        }
        return result;
      });
    }

    if (action == "replace_day_block") {
      const auto response = runtime.pipeline().RunReplaceTxtDayBlock(
          {.content = RequireStringField(payload, "content"),
           .day_marker = RequireStringField(payload, "day_marker"),
           .edited_day_body = RequireStringField(payload, "edited_day_body")});
      if (!response.ok) {
        return BuildFailureResponse(response.error_message);
      }
      return BuildTxtSuccessResponse([&]() -> json {
        return json{{"normalized_day_marker", response.normalized_day_marker},
                    {"found", response.found},
                    {"is_marker_valid", response.is_marker_valid},
                    {"updated_content", response.updated_content}};
      });
    }

    if (action == "convert_activity_names") {
      const auto response = runtime.pipeline().RunConvertTxtActivityNames(
          {.content = RequireStringField(payload, "content"),
           .direction = RequireStringField(payload, "direction")});
      if (!response.ok) {
        return BuildFailureResponse(response.error_message);
      }
      return BuildTxtSuccessResponse([&]() -> json {
        return json{{"converted_content", response.converted_content}};
      });
    }

    if (action == "replace_canonical_activity_names") {
      const auto replacements_it = payload.find("replacements");
      if (replacements_it == payload.end() || !replacements_it->is_array()) {
        throw std::invalid_argument("field `replacements` must be an array.");
      }
      std::vector<tracer_core::core::dto::CanonicalActivityNameReplacement>
          replacements;
      replacements.reserve(replacements_it->size());
      for (const auto& replacement : *replacements_it) {
        if (!replacement.is_object()) {
          throw std::invalid_argument(
              "each `replacements` item must be an object.");
        }
        replacements.push_back(
            {.old_canonical = RequireStringField(replacement, "old_canonical"),
             .new_canonical = RequireStringField(replacement, "new_canonical")});
      }
      const auto response = runtime.pipeline().RunReplaceTxtCanonicalActivityNames(
          {.content = RequireStringField(payload, "content"),
           .replacements = std::move(replacements)});
      if (!response.ok) {
        return BuildFailureResponse(response.error_message);
      }
      return BuildTxtSuccessResponse([&]() -> json {
        return json{{"updated_content", response.updated_content}};
      });
    }

    if (action == "replace_alias_activity_names") {
      const auto replacements_it = payload.find("replacements");
      if (replacements_it == payload.end() || !replacements_it->is_array()) {
        throw std::invalid_argument("field `replacements` must be an array.");
      }
      std::vector<tracer_core::core::dto::AliasActivityNameReplacement>
          replacements;
      replacements.reserve(replacements_it->size());
      for (const auto& replacement : *replacements_it) {
        if (!replacement.is_object()) {
          throw std::invalid_argument(
              "each `replacements` item must be an object.");
        }
        replacements.push_back(
            {.old_alias = RequireStringField(replacement, "old_alias"),
             .new_alias = RequireStringField(replacement, "new_alias")});
      }
      const auto response = runtime.pipeline().RunReplaceTxtAliasActivityNames(
          {.content = RequireStringField(payload, "content"),
           .replacements = std::move(replacements)});
      if (!response.ok) {
        return BuildFailureResponse(response.error_message);
      }
      return BuildTxtSuccessResponse([&]() -> json {
        return json{{"updated_content", response.updated_content}};
      });
    }

    if (action == "apply_activity_hierarchy_operation") {
      try {
        return BuildTxtSuccessResponse(
            [&]() { return ApplyActivityHierarchyOperationJson(payload); });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.activity_hierarchy.failed",
                                    "config",
                                    {"Inspect the hierarchy operation fields."});
      }
    }

    if (action == "move_activity_hierarchy_leaf_between_documents") {
      try {
        return BuildTxtSuccessResponse(
            [&]() { return MoveActivityHierarchyLeafBetweenDocumentsJson(payload); });
      } catch (const std::exception& error) {
        return BuildFailureResponse(
            error.what(), "config.activity_hierarchy.failed", "config",
            {"Inspect the source, destination, document set, and leaf operation fields."});
      }
    }

    if (action == "move_activity_hierarchy_node_between_documents") {
      try {
        return BuildTxtSuccessResponse(
            [&]() { return MoveActivityHierarchyNodeBetweenDocumentsJson(payload); });
      } catch (const std::exception& error) {
        return BuildFailureResponse(
            error.what(), "config.activity_hierarchy.failed", "config",
            {"Inspect the source, destination, document set, and move operation fields."});
      }
    }

    if (action == "rewrite_activity_hierarchy_document") {
      try {
        return BuildTxtSuccessResponse(
            [&]() { return RewriteActivityHierarchyDocumentJson(payload); });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.activity_hierarchy.failed",
                                    "config",
                                    {"Inspect the original and updated canonical TOML."});
      }
    }

    if (action == "describe_activity_hierarchy") {
      try {
        return BuildTxtSuccessResponse(
            [&]() { return DescribeActivityHierarchyJson(payload); });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.activity_hierarchy.failed",
                                    "config", {});
      }
    }

    if (action == "validate_activity_hierarchy_documents") {
      try {
        return BuildTxtSuccessResponse(
            [&]() { return ValidateActivityHierarchyDocumentsJson(payload); });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.activity_hierarchy.failed",
                                    "config", {});
      }
    }

    if (action == "render_activity_hierarchy_text") {
      try {
        const std::string toml_content =
            RequireStringField(payload, "toml_content");
        return BuildTxtSuccessResponse([&]() -> json {
          return json{{"content",
                       tracer::core::application::config::RenderActivityHierarchyText(
                           std::string_view(toml_content),
                           payload.value("show_aliases", false))}};
        });
      } catch (const std::exception& error) {
        return BuildFailureResponse(error.what(), "config.activity_hierarchy.failed",
                                    "config", {});
      }
    }

    return BuildFailureResponse(
        "Unsupported runtime config action: " + action,
        "runtime.invalid_request", "runtime",
        {"Use action=read_quick_access|write_quick_access|default_day_marker|resolve_day_block|replace_day_block|convert_activity_names|replace_canonical_activity_names|replace_alias_activity_names|apply_activity_hierarchy_operation|move_activity_hierarchy_leaf_between_documents|move_activity_hierarchy_node_between_documents|rewrite_activity_hierarchy_document|describe_activity_hierarchy|validate_activity_hierarchy_documents|render_activity_hierarchy_text."});
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_config_json failed unexpectedly.");
  }
}

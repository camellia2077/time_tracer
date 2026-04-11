import tracer.core.application.use_cases.interface;

#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

#include "nlohmann/json.hpp"
#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;
using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;

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

}  // namespace

extern "C" TT_CORE_API auto tracer_core_runtime_txt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  // TXT uses its own runtime family so hosts can reuse shared month-TXT
  // authoring semantics without routing these requests through query/pipeline.
  try {
    ClearLastError();
    ITracerCoreRuntime& runtime = RequireRuntime(handle);
    const json payload = json::parse(ToRequestJsonView(request_json));
    if (!payload.is_object()) {
      throw std::invalid_argument("request_json must be a JSON object.");
    }

    const std::string action = RequireStringField(payload, "action");
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

    return BuildFailureResponse(
        "Unsupported txt action: " + action, "runtime.invalid_request",
        "runtime", {"Use action=default_day_marker|resolve_day_block|replace_day_block."});
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_txt_json failed unexpectedly.");
  }
}

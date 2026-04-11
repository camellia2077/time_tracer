// api/c_api/tracer_core_c_api.cpp
#include "api/c_api/tracer_core_c_api.h"

#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "shared/types/version.hpp"

using tracer_core::core::c_api::internal::BuildCapabilitiesResponseJson;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::SetLastError;

#include "api/c_api/runtime/internal/tracer_core_c_api_namespace.inc"

extern "C" TT_CORE_API auto tracer_core_get_version(void) -> const char* {
  try {
    ClearLastError();
    return AppInfo::kVersion.data();
  } catch (...) {
    SetLastError("tracer_core_get_version failed unexpectedly.");
    return "";
  }
}

extern "C" TT_CORE_API auto tracer_core_ping(void) -> int {
  try {
    ClearLastError();
    return TT_CORE_STATUS_OK;
  } catch (...) {
    SetLastError("tracer_core_ping failed unexpectedly.");
    return TT_CORE_STATUS_ERROR;
  }
}

extern "C" TT_CORE_API auto tracer_core_get_capabilities_json(void) -> const
    char* {
  try {
    ClearLastError();
    return BuildCapabilitiesResponseJson();
  } catch (const std::exception& error) {
    SetLastError(error.what());
    return "{}";
  } catch (...) {
    SetLastError("tracer_core_get_capabilities_json failed unexpectedly.");
    return "{}";
  }
}

extern "C" TT_CORE_API auto tracer_core_get_build_info_json(void) -> const
    char* {
  try {
    ClearLastError();
    return BuildSuccessJsonResponse(
        json{{"ok", true},
             {"error_message", ""},
             {"core_version", std::string(AppInfo::kVersion)},
             {"abi_name", std::string(kCoreAbiName)},
             {"abi_version", kCoreAbiVersion},
             {"build_time_utc", std::string(AppInfo::kLastUpdated)}});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what(), {},
                                    "runtime.build_info_error", "runtime");
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_get_build_info_json failed unexpectedly.", {},
        "runtime.build_info_error", "runtime");
  }
}

extern "C" TT_CORE_API auto tracer_core_get_command_contract_json(
    const char* kRequest) -> const char* {
  try {
    ClearLastError();

    std::optional<std::string> command_filter;
    if (IsNonEmptyCString(kRequest)) {
      const json kRequestJson = json::parse(kRequest);
      if (!kRequestJson.is_object()) {
        return BuildFailureJsonResponse(
            "request_json must be a JSON object.", {},
            "contract.invalid_request", "contract",
            {R"(Use JSON object, e.g. {"command":"query"}.)"});
      }
      const auto kCommandIt = kRequestJson.find("command");
      if (kCommandIt != kRequestJson.end() && !kCommandIt->is_null()) {
        if (!kCommandIt->is_string()) {
          return BuildFailureJsonResponse(
              "field `command` must be a string.", {},
              "contract.invalid_request", "contract",
              {R"(Use JSON object, e.g. {"command":"query"}.)"});
        }
        command_filter = kCommandIt->get<std::string>();
      }
    }

    json commands =
        json::array({json{{"id", "blink"},
                          {"aliases", json::array({"ingest"})},
                          {"supports", json{{"structured_output", false}}}},
                     json{{"id", "ingest"},
                          {"aliases", json::array({"blink"})},
                          {"supports", json{{"structured_output", false}}}},
                     json{{"id", "query"},
                          {"aliases", json::array()},
                          {"supports", json{{"structured_output", true}}}},
                     json{{"id", "tree"},
                          {"aliases", json::array()},
                          {"supports", json{{"structured_output", true}}}},
                     json{{"id", "report"},
                          {"aliases", json::array()},
                          {"supports", json{{"structured_output", false}}}},
                     json{{"id", "txt"},
                          {"aliases", json::array()},
                          {"supports", json{{"structured_output", true}}}},
                     json{{"id", "export"},
                          {"aliases", json::array()},
                          {"supports", json{{"structured_output", false}}}},
                     json{{"id", "crypto"},
                          {"aliases", json::array()},
                          {"supports", json{{"structured_output", false}}}}});

    if (command_filter.has_value()) {
      json filtered = json::array();
      for (const auto& item : commands) {
        if (item.value("id", "") == *command_filter) {
          filtered.push_back(item);
          break;
        }
      }
      commands = std::move(filtered);
    }

    return BuildSuccessJsonResponse(json{{"ok", true},
                                         {"error_message", ""},
                                         {"contract_version", "1"},
                                         {"commands", std::move(commands)}});
  } catch (const json::parse_error& error) {
    return BuildFailureJsonResponse(
        std::string("Invalid request JSON: ") + error.what(), {},
        "contract.invalid_request", "contract",
        {R"(Use JSON object, e.g. {"command":"query"}.)"});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what(), {}, "contract.internal_error",
                                    "contract");
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_get_command_contract_json failed unexpectedly.", {},
        "contract.internal_error", "contract");
  }
}

extern "C" TT_CORE_API auto tracer_core_last_error(void) -> const char* {
  if (tracer_core::core::c_api::internal::g_last_error.empty()) {
    return "";
  }
  return tracer_core::core::c_api::internal::g_last_error.c_str();
}

extern "C" TT_CORE_API void tracer_core_set_log_callback(
    TtCoreLogCallback callback, void* user_data) {
  std::scoped_lock lock(g_callback_mutex);
  g_log_callback_registration.callback = callback;
  g_log_callback_registration.user_data = user_data;
}

extern "C" TT_CORE_API void tracer_core_set_diagnostics_callback(
    TtCoreDiagnosticsCallback callback, void* user_data) {
  std::scoped_lock lock(g_callback_mutex);
  g_diagnostics_callback_registration.callback = callback;
  g_diagnostics_callback_registration.user_data = user_data;
}

extern "C" TT_CORE_API void tracer_core_set_crypto_progress_callback(
    TtCoreCryptoProgressCallback callback, void* user_data) {
  tracer_core::core::c_api::internal::SetCryptoProgressCallbackRegistration(
      callback, user_data);
}

// ABI compatibility: parameter order is part of exported C contract.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
extern "C" TT_CORE_API auto tracer_core_runtime_create(
    const char* db_path, const char* output_root,
    const char* converter_config_toml_path) -> TtCoreRuntimeHandle* {
  try {
    ClearLastError();
    if (output_root == nullptr || output_root[0] == '\0') {
      throw std::invalid_argument("output_root must not be empty.");
    }
    if (converter_config_toml_path == nullptr ||
        converter_config_toml_path[0] == '\0') {
      throw std::invalid_argument(
          "converter_config_toml_path must not be empty.");
    }

    infrastructure::bootstrap::AndroidRuntimeRequest request{};
    if (db_path != nullptr && db_path[0] != '\0') {
      request.db_path = fs::path(db_path);
    }
    request.output_root = fs::path(output_root);
    request.converter_config_toml_path = fs::path(converter_config_toml_path);
    request.logger = CreateAbiLoggerAdapter();
    request.diagnostics_sink = CreateAbiDiagnosticsAdapter();

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.runtime_api) {
      throw std::runtime_error("failed to create core runtime.");
    }

    auto* handle = new TtCoreRuntimeHandle();
    handle->runtime = std::move(runtime);
    handle->output_root = fs::absolute(request.output_root);
    handle->converter_config_toml_path =
        fs::absolute(request.converter_config_toml_path);
    return handle;
  } catch (const std::exception& error) {
    SetLastError(error.what());
    return nullptr;
  } catch (...) {
    SetLastError("tracer_core_runtime_create failed unexpectedly.");
    return nullptr;
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

extern "C" TT_CORE_API void tracer_core_runtime_destroy(
    TtCoreRuntimeHandle* handle) {
  delete handle;
}

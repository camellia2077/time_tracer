// api/core_c/tracer_core_c_api.cpp
#include "api/core_c/tracer_core_c_api.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "api/core_c/tracer_core_c_api_internal.hpp"
#include "infrastructure/config/config_loader.hpp"
#include "infrastructure/config/models/app_config.hpp"
#include "shared/types/version.hpp"

using tracer_core::core::c_api::internal::BuildCapabilitiesResponseJson;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::SetLastError;

#include "api/core_c/internal/tracer_core_c_api_namespace.inc"


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

extern "C" TT_CORE_API auto tracer_core_runtime_check_environment_json(
    const char* executable_path, int is_help_mode) -> const char* {
  try {
    ClearLastError();
    const ResolvedCliContext kContext =
        ResolveCliContext(executable_path, nullptr, nullptr, nullptr);
    if (is_help_mode != 0) {
      return BuildSuccessJsonResponse(json{
          {"ok", true},
          {"error_message", ""},
          {"messages", json::array()},
      });
    }

    const fs::path kBinDir = kContext.app_config.exe_dir_path;
    std::vector<std::string> errors;
    std::vector<fs::path> required_files = {
        kBinDir / kCoreLibraryName,
        kBinDir / kReportsSharedLibraryName,
        kBinDir / "config" / "config.toml",
    };
    if (TT_RUNTIME_REQUIRE_SQLITE_DLL != 0) {
      required_files.emplace_back(kBinDir / kSqliteLibraryName);
    }
    if (TT_RUNTIME_REQUIRE_TOML_DLL != 0) {
      required_files.emplace_back(kBinDir / kTomlLibraryName);
    }
    if (TT_RUNTIME_REQUIRE_MINGW_DLLS != 0) {
      required_files.emplace_back(kBinDir / kLibgccRuntimeName);
      required_files.emplace_back(kBinDir / kLibstdcppRuntimeName);
      required_files.emplace_back(kBinDir / kLibwinpthreadRuntimeName);
    }

    for (const auto& path : required_files) {
      if (!fs::exists(path)) {
        errors.emplace_back("[runtime-check] missing required runtime file: " +
                            path.string());
      }
    }

    if (tracer_core_ping() != TT_CORE_STATUS_OK) {
      const char* error = tracer_core_last_error();
      errors.emplace_back(std::string("[runtime-check] core ping failed: ") +
                          (error != nullptr ? error : "unknown"));
    }

    const char* capabilities_json = tracer_core_get_capabilities_json();
    if (capabilities_json == nullptr || capabilities_json[0] == '\0') {
      errors.emplace_back(
          "[runtime-check] tracer_core_get_capabilities_json returned empty "
          "payload.");
    } else {
      try {
        const json kCapabilities = json::parse(capabilities_json);
        const auto kFeaturesIt = kCapabilities.find("features");
        if (kFeaturesIt == kCapabilities.end() || !kFeaturesIt->is_object()) {
          errors.emplace_back(
              "[runtime-check] capabilities payload missing `features` "
              "object.");
        }
      } catch (const std::exception& error) {
        errors.emplace_back(std::string("[runtime-check] invalid capabilities "
                                        "payload: ") +
                            error.what());
      }
    }

    if (!errors.empty()) {
      return BuildFailureJsonResponse(errors.front(), errors);
    }
    return BuildSuccessJsonResponse(
        json{{"ok", true}, {"error_message", ""}, {"messages", json::array()}});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what());
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_runtime_check_environment_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_resolve_cli_context_json(
    const char* executable_path, const char* db_override,
    const char* output_override, const char* command_name) -> const char* {
  try {
    ClearLastError();
    const ResolvedCliContext kContext = ResolveCliContext(
        executable_path, db_override, output_override, command_name);
    return BuildSuccessJsonResponse(
        json{{"ok", true},
             {"error_message", ""},
             {"paths",
              {{"exe_dir", kContext.app_config.exe_dir_path.string()},
               {"db_path", kContext.db_path.string()},
               {"output_root", kContext.output_root.string()},
               {"export_root", kContext.export_root.string()},
               {"runtime_output_root", kContext.runtime_output_root.string()},
               {"converter_config_toml_path",
                kContext.app_config.pipeline.interval_processor_config_path
                    .string()}}},
             {"cli_config", BuildCliConfigJson(kContext.app_config)}});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what(), {}, "config.resolve_failed",
                                    "config");
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_runtime_resolve_cli_context_json failed unexpectedly.", {},
        "config.resolve_failed", "config");
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
    if (!runtime.core_api) {
      throw std::runtime_error("failed to create core runtime.");
    }

    auto* handle = new TtCoreRuntimeHandle();
    handle->runtime = std::move(runtime);
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

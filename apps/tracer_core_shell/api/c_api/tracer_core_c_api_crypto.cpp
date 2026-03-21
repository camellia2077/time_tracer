#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/tracer_core_c_api_internal.hpp"
#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_tracer_core_runtime.hpp"
#include "host/crypto_progress_bridge.hpp"
#include "host/tracer_exchange_inspect_formatter.hpp"
#include "nlohmann/json.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseDateCheckMode;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;

namespace {

namespace fs = std::filesystem;
namespace app_dto = tracer_core::core::dto;
using nlohmann::json;

constexpr std::string_view kProducerPlatform = "windows";
constexpr std::string_view kProducerApp = "time_tracer_cli";

[[nodiscard]] auto ResolveActiveTextRootPath(
    const fs::path& active_converter_main_config_path) -> fs::path {
  const fs::path main_config = fs::absolute(active_converter_main_config_path);
  const fs::path runtime_root =
      main_config.parent_path().parent_path().parent_path();
  return runtime_root / "input" / "full";
}

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) -> char {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

[[nodiscard]] auto ParseJsonRequest(std::string_view request_json) -> json {
  if (request_json.empty()) {
    throw std::invalid_argument("request_json must not be empty.");
  }
  try {
    return json::parse(request_json);
  } catch (const std::exception& error) {
    throw std::invalid_argument(std::string("Invalid request JSON: ") +
                                error.what());
  }
}

[[nodiscard]] auto RequireStringField(const json& payload, std::string_view key)
    -> std::string {
  const std::string key_text(key);
  const auto it = payload.find(key_text);
  if (it == payload.end() || !it->is_string() ||
      it->get_ref<const std::string&>().empty()) {
    throw std::invalid_argument("field `" + key_text +
                                "` must be a non-empty string.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto OptionalStringField(const json& payload,
                                       std::string_view key)
    -> std::optional<std::string> {
  const std::string key_text(key);
  const auto it = payload.find(key_text);
  if (it == payload.end() || it->is_null()) {
    return std::nullopt;
  }
  if (!it->is_string()) {
    throw std::invalid_argument("field `" + key_text + "` must be a string.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto OptionalBoolField(const json& payload, std::string_view key)
    -> std::optional<bool> {
  const std::string key_text(key);
  const auto it = payload.find(key_text);
  if (it == payload.end() || it->is_null()) {
    return std::nullopt;
  }
  if (!it->is_boolean()) {
    throw std::invalid_argument("field `" + key_text + "` must be a boolean.");
  }
  return it->get<bool>();
}

auto RejectFieldIfPresent(const json& payload, std::string_view key,
                          std::string_view message) -> void {
  if (payload.contains(std::string(key))) {
    throw std::invalid_argument(std::string(message));
  }
}

[[nodiscard]] auto ParseSecurityLevel(const std::optional<std::string>& token)
    -> app_dto::TracerExchangeSecurityLevel {
  const std::string normalized = ToLowerAscii(token.value_or("interactive"));
  if (normalized == "min") {
    return app_dto::TracerExchangeSecurityLevel::kMin;
  }
  if (normalized == "interactive") {
    return app_dto::TracerExchangeSecurityLevel::kInteractive;
  }
  if (normalized == "moderate") {
    return app_dto::TracerExchangeSecurityLevel::kModerate;
  }
  if (normalized == "high" || normalized == "sensitive") {
    return app_dto::TracerExchangeSecurityLevel::kHigh;
  }
  if (normalized == "max") {
    return app_dto::TracerExchangeSecurityLevel::kMax;
  }
  throw std::invalid_argument(
      "field `security_level` must be one of: "
      "min|interactive|moderate|high|max (alias: sensitive).");
}

[[nodiscard]] auto EmitCryptoProgress(
    const app_dto::TracerExchangeProgressSnapshot& snapshot)
    -> app_dto::TracerExchangeProgressControl {
  const auto registration =
      tracer_core::core::c_api::internal::GetCryptoProgressCallbackRegistration();
  if (registration.callback == nullptr) {
    return app_dto::TracerExchangeProgressControl::kContinue;
  }

  const std::string progress_json =
      tracer_core::shell::crypto_progress_bridge::BuildProgressSnapshotJson(
          snapshot);
  try {
    registration.callback(progress_json.c_str(), registration.user_data);
  } catch (...) {
    // Host callback failures must not interrupt core execution.
  }
  return app_dto::TracerExchangeProgressControl::kContinue;
}

[[nodiscard]] auto BuildProgressObserver()
    -> app_dto::TracerExchangeProgressObserver {
  return [](const app_dto::TracerExchangeProgressSnapshot& snapshot) {
    return EmitCryptoProgress(snapshot);
  };
}

[[nodiscard]] auto BuildEncryptTextOutput(
    const app_dto::TracerExchangeExportResult& result,
    const app_dto::TracerExchangeExportRequest& request)
    -> app_dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }
  return {
      .ok = true,
      .content = "Exported complete tracer exchange package: " +
                 fs::absolute(request.input_text_root_path).string() + " -> " +
                 result.resolved_output_tracer_path.string() + "\n"
                 "Included payload TXT files: " +
                 std::to_string(result.payload_file_count) + "\n"
                 "Included converter TOML files: " +
                 std::to_string(result.converter_file_count) + "\n"
                 "Manifest included: " +
                 std::string(result.manifest_included ? "yes" : "no"),
      .error_message = "",
  };
}

[[nodiscard]] auto BuildImportTextOutput(
    const app_dto::TracerExchangeImportResult& result,
    const app_dto::TracerExchangeImportRequest& request)
    -> app_dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }

  std::string content = "Imported complete tracer exchange package: " +
                        fs::absolute(request.input_tracer_path).string() + "\n"
                        "Updated active text root: " +
                        request.active_text_root_path.string() + "\n"
                        "Applied converter config: " +
                        std::string(result.config_applied ? "yes" : "no") + "\n"
                        "Replaced months: " +
                        std::to_string(result.replaced_month_count) + "\n"
                        "Preserved local months: " +
                        std::to_string(result.preserved_month_count) + "\n"
                        "Rebuilt months: " +
                        std::to_string(result.rebuilt_month_count) + "\n"
                        "Database rebuilt: " +
                        std::string(result.database_rebuilt ? "yes" : "no");
  if (result.backup_retained_root.has_value()) {
    content += "\nRetained backup root: " + result.backup_retained_root->string();
    if (!result.backup_cleanup_error.empty()) {
      content += " | " + result.backup_cleanup_error;
    }
  } else {
    if (!result.backup_cleanup_error.empty()) {
      content += "\nCleanup warning: " + result.backup_cleanup_error;
    }
  }

  return {.ok = true, .content = std::move(content), .error_message = ""};
}

[[nodiscard]] auto BuildInspectTextOutput(
    const app_dto::TracerExchangeInspectResult& result) -> app_dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }
  return {
      .ok = true,
      .content = tracer_core::shell::tracer_exchange::BuildInspectContent(result),
      .error_message = "",
  };
}

[[nodiscard]] auto RunCryptoEncrypt(TtCoreRuntimeHandle* handle,
                                    std::string_view request_json)
    -> app_dto::TextOutput {
  const json payload = ParseJsonRequest(request_json);
  ITracerCoreRuntime& runtime = RequireRuntime(handle);

  DateCheckMode date_check_mode = DateCheckMode::kNone;
  if (const auto mode = OptionalStringField(payload, "date_check_mode");
      mode.has_value()) {
    date_check_mode = ParseDateCheckMode(*mode);
  }

  const app_dto::TracerExchangeExportRequest request{
      .input_text_root_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .requested_output_path =
          fs::absolute(fs::path(RequireStringField(payload, "output_path"))),
      .active_converter_main_config_path = handle->converter_config_toml_path,
      .date_check_mode = date_check_mode,
      .passphrase = RequireStringField(payload, "passphrase"),
      .producer_platform = std::string(kProducerPlatform),
      .producer_app = std::string(kProducerApp),
      .security_level =
          ParseSecurityLevel(OptionalStringField(payload, "security_level")),
      .progress_observer = BuildProgressObserver(),
  };
  return BuildEncryptTextOutput(
      runtime.tracer_exchange().RunTracerExchangeExport(request), request);
}

[[nodiscard]] auto RunCryptoDecrypt(TtCoreRuntimeHandle* handle,
                                    std::string_view request_json)
    -> app_dto::TextOutput {
  const json payload = ParseJsonRequest(request_json);
  ITracerCoreRuntime& runtime = RequireRuntime(handle);
  const auto output_path = OptionalStringField(payload, "output_path");
  RejectFieldIfPresent(payload, "apply_config",
                       "field `apply_config` is no longer supported for "
                       "tracer exchange import.");
  if (output_path.has_value() && output_path->empty()) {
    throw std::invalid_argument(
        "field `output_path` must be a non-empty string when provided.");
  }

  const app_dto::TracerExchangeImportRequest request{
      .input_tracer_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .active_text_root_path =
          ResolveActiveTextRootPath(handle->converter_config_toml_path),
      .active_converter_main_config_path = handle->converter_config_toml_path,
      .runtime_work_root = output_path.has_value()
                               ? fs::absolute(fs::path(*output_path))
                               : handle->output_root,
      .passphrase = RequireStringField(payload, "passphrase"),
      .progress_observer = BuildProgressObserver(),
  };
  return BuildImportTextOutput(
      runtime.tracer_exchange().RunTracerExchangeImport(request), request);
}

[[nodiscard]] auto RunCryptoInspect(TtCoreRuntimeHandle* handle,
                                    std::string_view request_json)
    -> app_dto::TextOutput {
  const json payload = ParseJsonRequest(request_json);
  ITracerCoreRuntime& runtime = RequireRuntime(handle);
  RejectFieldIfPresent(payload, "output_path",
                       "field `output_path` is not supported for crypto "
                       "inspect.");

  const app_dto::TracerExchangeInspectRequest request{
      .input_tracer_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .passphrase = RequireStringField(payload, "passphrase"),
      .progress_observer = BuildProgressObserver(),
  };
  return BuildInspectTextOutput(
      runtime.tracer_exchange().RunTracerExchangeInspect(request));
}

}  // namespace

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_encrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    return BuildTextResponse(
        RunCryptoEncrypt(handle, ToRequestJsonView(request_json)));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_crypto_encrypt_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_decrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    return BuildTextResponse(
        RunCryptoDecrypt(handle, ToRequestJsonView(request_json)));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_crypto_decrypt_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_inspect_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    return BuildTextResponse(
        RunCryptoInspect(handle, ToRequestJsonView(request_json)));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_crypto_inspect_json failed unexpectedly.");
  }
}

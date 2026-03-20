#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

#include "api/android_jni/native_bridge_internal.hpp"
#include "api/c_api/c_api_parse_bridge.hpp"
#include "api/c_api/tracer_core_c_api_internal.hpp"
#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "host/native_bridge_crypto_helpers.hpp"
#include "host/tracer_exchange_inspect_formatter.hpp"

namespace tracer_core::api::android::bridge_internal {

namespace fs = std::filesystem;
namespace app_dto = tracer_core::core::dto;
using nlohmann::json;
using tracer::core::application::use_cases::ITracerCoreApi;

namespace {

constexpr std::string_view kProducerPlatform = "android";
constexpr std::string_view kProducerApp = "time_tracer_android";

auto ResolveActiveTextRootPath(
    const fs::path& active_converter_main_config_path) -> fs::path {
  const fs::path main_config = fs::absolute(active_converter_main_config_path);
  const fs::path runtime_root =
      main_config.parent_path().parent_path().parent_path();
  return runtime_root / "input" / "full";
}

auto ParseTracerExchangeSecurityLevel(std::string_view value)
    -> std::optional<app_dto::TracerExchangeSecurityLevel> {
  const auto parsed_crypto_level = ParseCryptoSecurityLevel(value);
  if (!parsed_crypto_level.has_value()) {
    return std::nullopt;
  }

  switch (parsed_crypto_level.value()) {
    case tracer_core::infrastructure::crypto::FileCryptoSecurityLevel::
        kInteractive:
      return app_dto::TracerExchangeSecurityLevel::kInteractive;
    case tracer_core::infrastructure::crypto::FileCryptoSecurityLevel::
        kModerate:
      return app_dto::TracerExchangeSecurityLevel::kModerate;
    case tracer_core::infrastructure::crypto::FileCryptoSecurityLevel::kHigh:
      return app_dto::TracerExchangeSecurityLevel::kHigh;
    case tracer_core::infrastructure::crypto::FileCryptoSecurityLevel::kMin:
      return app_dto::TracerExchangeSecurityLevel::kMin;
    case tracer_core::infrastructure::crypto::FileCryptoSecurityLevel::kMax:
      return app_dto::TracerExchangeSecurityLevel::kMax;
  }

  return std::nullopt;
}

auto BuildExportContentJson(const app_dto::TracerExchangeExportResult& result)
    -> std::string {
  return json{
      {"output_path", result.resolved_output_tracer_path.string()},
      {"source_root_name", result.source_root_name},
      {"payload_file_count", result.payload_file_count},
      {"converter_file_count", result.converter_file_count},
      {"manifest_included", result.manifest_included},
  }
      .dump();
}

auto BuildImportContentJson(const app_dto::TracerExchangeImportResult& result)
    -> std::string {
  return json{
      {"source_root_name", result.source_root_name},
      {"payload_file_count", result.payload_file_count},
      {"replaced_month_count", result.replaced_month_count},
      {"preserved_month_count", result.preserved_month_count},
      {"rebuilt_month_count", result.rebuilt_month_count},
      {"text_root_updated", result.text_root_updated},
      {"config_applied", result.config_applied},
      {"database_rebuilt", result.database_rebuilt},
      {"retained_failure_root",
       result.retained_failure_root.has_value()
           ? result.retained_failure_root->string()
           : std::string()},
      {"backup_retained_root",
       result.backup_retained_root.has_value()
           ? result.backup_retained_root->string()
           : std::string()},
      {"backup_cleanup_error", result.backup_cleanup_error},
  }
      .dump();
}

auto BuildInspectContentJson(const app_dto::TracerExchangeInspectResult& result)
    -> std::string {
  return json{
      {"input_path", result.input_tracer_path.string()},
      {"package_type", result.package_type},
      {"package_version", result.package_version},
      {"source_root_name", result.source_root_name},
      {"payload_file_count", result.payload_file_count},
      {"producer_platform", result.producer_platform},
      {"producer_app", result.producer_app},
      {"created_at_utc", result.created_at_utc},
      {"rendered_text",
       tracer_core::shell::tracer_exchange::BuildInspectContent(result)},
  }
      .dump();
}

auto ParseAndroidDateCheckMode(jint value) -> DateCheckMode {
  return tracer_core::shell::c_api_bridge::ParseDateCheckMode(
      ParseDateCheckMode(value));
}

}  // namespace

auto NativeEncryptFile(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                       jstring output_path, jstring passphrase,
                       jstring security_level) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    const std::string output_path_utf8 = ToUtf8(env, output_path);
    const std::string passphrase_utf8 = ToUtf8(env, passphrase);
    const std::string security_level_utf8 = ToUtf8(env, security_level);

    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }
    if (output_path_utf8.empty()) {
      return BuildResponseJson(false, "outputPath must not be empty.",
                               std::string{});
    }
    if (passphrase_utf8.empty()) {
      return BuildResponseJson(false, "passphrase must not be empty.",
                               std::string{});
    }

    const auto parsed_security_level =
        ParseCryptoSecurityLevel(security_level_utf8);
    if (!parsed_security_level.has_value()) {
      return BuildResponseJson(
          false, "securityLevel must be one of: interactive, moderate, high.",
          std::string{});
    }

    auto options = BuildCryptoOptions(env);
    options.security_level = parsed_security_level.value();
    const auto result = tracer_core::infrastructure::crypto::EncryptFile(
        std::filesystem::path(input_path_utf8),
        std::filesystem::path(output_path_utf8), passphrase_utf8, options);
    if (!result.ok()) {
      const std::string error_message = !result.error_message.empty()
                                            ? result.error_message
                                            : result.error_code;
      const std::string content_json =
          json{
              {"error_code", result.error_code},
          }
              .dump();
      return BuildResponseJson(false, error_message, content_json);
    }

    const std::string content_json =
        json{
            {"input_path", input_path_utf8},
            {"output_path", output_path_utf8},
        }
            .dump();
    return BuildResponseJson(true, std::string{}, content_json);
  });
}

auto NativeExportTracerExchange(JNIEnv* env, jobject /*thiz*/,
                                jstring input_path, jstring output_path,
                                jstring passphrase, jstring security_level,
                                jint date_check_mode) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    const std::string output_path_utf8 = ToUtf8(env, output_path);
    const std::string passphrase_utf8 = ToUtf8(env, passphrase);
    const std::string security_level_utf8 = ToUtf8(env, security_level);

    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }
    if (output_path_utf8.empty()) {
      return BuildResponseJson(false, "outputPath must not be empty.",
                               std::string{});
    }
    if (passphrase_utf8.empty()) {
      return BuildResponseJson(false, "passphrase must not be empty.",
                               std::string{});
    }

    const auto parsed_security_level =
        ParseTracerExchangeSecurityLevel(security_level_utf8);
    if (!parsed_security_level.has_value()) {
      return BuildResponseJson(
          false, "securityLevel must be one of: interactive, moderate, high.",
          std::string{});
    }

    app_dto::TracerExchangeExportResult result{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr ||
          g_runtime.core_runtime->runtime.core_api == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }

      ITracerCoreApi& runtime = *g_runtime.core_runtime->runtime.core_api;
      const app_dto::TracerExchangeExportRequest request{
          .input_text_root_path = fs::absolute(fs::path(input_path_utf8)),
          .requested_output_path = fs::absolute(fs::path(output_path_utf8)),
          .active_converter_main_config_path =
              g_runtime.core_runtime->converter_config_toml_path,
          .date_check_mode = ParseAndroidDateCheckMode(date_check_mode),
          .passphrase = passphrase_utf8,
          .producer_platform = std::string(kProducerPlatform),
          .producer_app = std::string(kProducerApp),
          .security_level = parsed_security_level.value(),
          .progress_observer = BuildTracerExchangeProgressObserver(env),
      };
      result = runtime.RunTracerExchangeExport(request);
    }

    if (!result.ok) {
      return BuildResponseJson(
          false,
          result.error_message.empty()
              ? "complete exchange package export failed."
              : result.error_message,
          std::string{});
    }
    return BuildResponseJson(true, std::string{}, BuildExportContentJson(result));
  });
}

auto NativeImportTracerExchange(JNIEnv* env, jobject /*thiz*/,
                                jstring input_path, jstring work_root,
                                jstring passphrase)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    const std::string work_root_utf8 = ToUtf8(env, work_root);
    const std::string passphrase_utf8 = ToUtf8(env, passphrase);

    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }
    if (work_root_utf8.empty()) {
      return BuildResponseJson(false, "workRoot must not be empty.",
                               std::string{});
    }
    if (passphrase_utf8.empty()) {
      return BuildResponseJson(false, "passphrase must not be empty.",
                               std::string{});
    }

    app_dto::TracerExchangeImportResult result{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr ||
          g_runtime.core_runtime->runtime.core_api == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }

      ITracerCoreApi& runtime = *g_runtime.core_runtime->runtime.core_api;
      const app_dto::TracerExchangeImportRequest request{
          .input_tracer_path = fs::absolute(fs::path(input_path_utf8)),
          .active_text_root_path = ResolveActiveTextRootPath(
              g_runtime.core_runtime->converter_config_toml_path),
          .active_converter_main_config_path =
              g_runtime.core_runtime->converter_config_toml_path,
          .runtime_work_root = fs::absolute(fs::path(work_root_utf8)),
          .passphrase = passphrase_utf8,
          .progress_observer = BuildTracerExchangeProgressObserver(env),
      };
      result = runtime.RunTracerExchangeImport(request);
    }

    if (!result.ok) {
      return BuildResponseJson(
          false,
          result.error_message.empty() ? "import tracer exchange failed."
                                       : result.error_message,
          BuildImportContentJson(result));
    }
    return BuildResponseJson(true, std::string{}, BuildImportContentJson(result));
  });
}

auto NativeInspectTracerExchange(JNIEnv* env, jobject /*thiz*/,
                                 jstring input_path, jstring passphrase)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    const std::string passphrase_utf8 = ToUtf8(env, passphrase);

    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }
    if (passphrase_utf8.empty()) {
      return BuildResponseJson(false, "passphrase must not be empty.",
                               std::string{});
    }

    app_dto::TracerExchangeInspectResult result{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr ||
          g_runtime.core_runtime->runtime.core_api == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }

      ITracerCoreApi& runtime = *g_runtime.core_runtime->runtime.core_api;
      const app_dto::TracerExchangeInspectRequest request{
          .input_tracer_path = fs::absolute(fs::path(input_path_utf8)),
          .passphrase = passphrase_utf8,
          .progress_observer = BuildTracerExchangeProgressObserver(env),
      };
      result = runtime.RunTracerExchangeInspect(request);
    }

    if (!result.ok) {
      return BuildResponseJson(
          false,
          result.error_message.empty() ? "inspect tracer exchange failed."
                                       : result.error_message,
          std::string{});
    }
    return BuildResponseJson(true, std::string{},
                             BuildInspectContentJson(result));
  });
}

auto NativeDecryptFile(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                       jstring output_path, jstring passphrase) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    const std::string output_path_utf8 = ToUtf8(env, output_path);
    const std::string passphrase_utf8 = ToUtf8(env, passphrase);

    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }
    if (output_path_utf8.empty()) {
      return BuildResponseJson(false, "outputPath must not be empty.",
                               std::string{});
    }
    if (passphrase_utf8.empty()) {
      return BuildResponseJson(false, "passphrase must not be empty.",
                               std::string{});
    }

    const auto options = BuildCryptoOptions(env);
    const auto result = tracer_core::infrastructure::crypto::DecryptFile(
        std::filesystem::path(input_path_utf8),
        std::filesystem::path(output_path_utf8), passphrase_utf8, options);
    if (!result.ok()) {
      const std::string error_message = !result.error_message.empty()
                                            ? result.error_message
                                            : result.error_code;
      const std::string content_json =
          json{
              {"error_code", result.error_code},
          }
              .dump();
      return BuildResponseJson(false, error_message, content_json);
    }

    const std::string content_json =
        json{
            {"input_path", input_path_utf8},
            {"output_path", output_path_utf8},
        }
            .dump();
    return BuildResponseJson(true, std::string{}, content_json);
  });
}

}  // namespace tracer_core::api::android::bridge_internal

// api/android/native_bridge_crypto_calls.cpp
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

#include "api/android/native_bridge_crypto_helpers.hpp"
#include "api/android/native_bridge_internal.hpp"
#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::api::android::bridge_internal {

using nlohmann::json;

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

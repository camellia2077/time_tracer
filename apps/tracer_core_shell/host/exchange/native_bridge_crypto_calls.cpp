#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/android_jni/native_bridge_internal.hpp"
#include "api/c_api/runtime/c_api_parse_bridge.hpp"
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/exchange_requests.hpp"
#include "application/dto/exchange_responses.hpp"
#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"
#include "host/exchange/native_bridge_crypto_helpers.hpp"
#include "host/exchange/tracer_exchange_inspect_formatter.hpp"

namespace tracer_core::api::android::bridge_internal {

namespace fs = std::filesystem;
namespace app_dto = tracer_core::core::dto;
namespace file_crypto = tracer_core::infrastructure::crypto;
using nlohmann::json;
using tracer::core::application::use_cases::ITracerCoreRuntime;

namespace {

#include "host/exchange/internal/native_bridge_crypto_calls_parse_impl.inc"

// JSON content and top-level bridge responses.
#include "host/exchange/internal/native_bridge_crypto_calls_response_impl.inc"

// Tracer exchange DTO construction plus runtime-locked dispatch.
#include "host/exchange/internal/native_bridge_crypto_calls_exchange_dispatch_impl.inc"

// Direct file encrypt/decrypt execution helpers.
#include "host/exchange/internal/native_bridge_crypto_calls_file_ops_impl.inc"

}  // namespace

auto NativeEncryptFile(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                       jstring output_path, jstring passphrase,
                       jstring security_level) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const EncryptFileArgs args = ParseEncryptFileArgs(
        env, input_path, output_path, passphrase, security_level);
    return BuildFileCryptoResponse(RunEncryptFileOperation(env, args),
                                   args.input_path, args.output_path);
  });
}

auto NativeExportTracerExchange(JNIEnv* env, jobject /*thiz*/,
                                jstring input_path, jstring output_path,
                                jstring passphrase, jstring security_level,
                                jint date_check_mode) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const ExportTracerExchangeArgs args =
        ParseExportTracerExchangeArgs(env, input_path, output_path, passphrase,
                                      security_level, date_check_mode);
    return BuildTracerExchangeExportResponse(
        DispatchExportTracerExchange(env, args));
  });
}

auto NativeImportTracerExchange(JNIEnv* env, jobject /*thiz*/,
                                jstring input_path, jstring work_root,
                                jstring passphrase) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const ImportTracerExchangeArgs args =
        ParseImportTracerExchangeArgs(env, input_path, work_root, passphrase);
    return BuildTracerExchangeImportResponse(
        DispatchImportTracerExchange(env, args));
  });
}

auto NativeInspectTracerExchange(JNIEnv* env, jobject /*thiz*/,
                                 jstring input_path, jstring passphrase)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const InspectTracerExchangeArgs args =
        ParseInspectTracerExchangeArgs(env, input_path, passphrase);
    return BuildTracerExchangeInspectResponse(
        DispatchInspectTracerExchange(env, args));
  });
}

auto NativeDecryptFile(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                       jstring output_path, jstring passphrase) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const DecryptFileArgs args =
        ParseDecryptFileArgs(env, input_path, output_path, passphrase);
    return BuildFileCryptoResponse(RunDecryptFileOperation(env, args),
                                   args.input_path, args.output_path);
  });
}

}  // namespace tracer_core::api::android::bridge_internal

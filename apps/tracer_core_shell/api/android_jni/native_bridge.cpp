// api/android_jni/native_bridge.cpp
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/android_jni/native_bridge_internal.hpp"
#include "api/android_jni/jni_runtime_code_bridge.hpp"
#include "jni/bridge_utils.hpp"

namespace tracer_core::api::android::bridge_internal {

std::mutex g_runtime_mutex;
RuntimeHolder g_runtime;
// Keep tracer_core_runtime_create marker in this TU for suite guard;
// actual runtime creation happens in native_bridge_calls.cpp.
[[maybe_unused]] constexpr std::string_view kCoreRuntimeCreateSymbol =
    "tracer_core_runtime_create";

[[nodiscard]] auto ToUtf8(JNIEnv* env, jstring text) -> std::string {
  if (text == nullptr) {
    return "";
  }
  const char* utf_chars = env->GetStringUTFChars(text, nullptr);
  if (utf_chars == nullptr) {
    throw std::runtime_error("Failed to access Java UTF-8 string.");
  }
  std::string output(utf_chars);
  env->ReleaseStringUTFChars(text, utf_chars);
  return output;
}

[[nodiscard]] auto ToJString(JNIEnv* env, std::string_view text) -> jstring {
  return env->NewStringUTF(std::string(text).c_str());
}

[[nodiscard]] auto BuildResponseJson(bool ok, std::string_view error_message,
                                     std::string_view content) -> std::string {
  return tracer_core_bridge_common::jni::BuildResponseJson(ok, error_message,
                                                           content);
}

auto DestroyRuntimeLocked() -> void {
  if (g_runtime.core_runtime != nullptr) {
    tracer_core_runtime_destroy(g_runtime.core_runtime);
    g_runtime.core_runtime = nullptr;
  }
}

[[nodiscard]] auto ReadOptionalText(JNIEnv* env, jstring text)
    -> std::optional<std::string> {
  const std::string value = ToUtf8(env, text);
  if (value.empty()) {
    return std::nullopt;
  }
  return value;
}

[[nodiscard]] auto ToIntVector(JNIEnv* env, jintArray values)
    -> std::vector<int> {
  if (values == nullptr) {
    return {};
  }

  const jsize length = env->GetArrayLength(values);
  std::vector<jint> buffer(static_cast<size_t>(length));
  if (length > 0) {
    env->GetIntArrayRegion(values, 0, length, buffer.data());
  }

  std::vector<int> output;
  output.reserve(buffer.size());
  for (const jint value : buffer) {
    output.push_back(static_cast<int>(value));
  }
  return output;
}

[[nodiscard]] auto ParseDateCheckMode(jint value) -> std::string {
  return tracer_core::shell::jni_bridge::ParseDateCheckModeCode(
      static_cast<int>(value));
}

[[nodiscard]] auto ParseRecordTimeOrderMode(jint value) -> std::string {
  return tracer_core::shell::jni_bridge::ParseRecordTimeOrderModeCode(
      static_cast<int>(value));
}

[[nodiscard]] auto ParseDataQueryAction(jint value) -> std::string {
  return tracer_core::shell::jni_bridge::ParseDataQueryActionCode(
      static_cast<int>(value));
}

[[nodiscard]] auto ParseReportType(jint value) -> std::string {
  return tracer_core::shell::jni_bridge::ParseReportTypeCode(
      static_cast<int>(value));
}

[[nodiscard]] auto ParseReportFormat(jint value) -> std::string {
  return tracer_core::shell::jni_bridge::ParseReportFormatCode(
      static_cast<int>(value));
}

[[nodiscard]] auto ParseCoreResponse(const char* response_json,
                                     std::string_view context)
    -> tt_transport::ResponseEnvelope {
  return tracer_core_bridge_common::jni::ParseCoreResponse(response_json,
                                                           context);
}

}  // namespace tracer_core::api::android::bridge_internal

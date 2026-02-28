// api/android/native_bridge.cpp
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/android/native_bridge_internal.hpp"

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
  return tt_transport::SerializeResponseEnvelope(
      tt_transport::BuildResponseEnvelope(ok, error_message, content));
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
  if (value == 0) {
    return "none";
  }
  if (value == 1) {
    return "continuity";
  }
  if (value == 2) {
    return "full";
  }
  throw std::invalid_argument("Unsupported date_check_mode code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseDataQueryAction(jint value) -> std::string {
  if (value == 0) {
    return "years";
  }
  if (value == 1) {
    return "months";
  }
  if (value == 2) {
    return "days";
  }
  if (value == 3) {
    return "days_duration";
  }
  if (value == 4) {
    return "days_stats";
  }
  if (value == 5) {
    return "search";
  }
  if (value == 6) {
    return "activity_suggest";
  }
  if (value == 7) {
    return "tree";
  }
  if (value == 8) {
    return "mapping_names";
  }
  if (value == 9) {
    return "report_chart";
  }
  throw std::invalid_argument("Unsupported query action code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportType(jint value) -> std::string {
  if (value == 0) {
    return "day";
  }
  if (value == 1) {
    return "month";
  }
  if (value == 2) {
    return "recent";
  }
  if (value == 3) {
    return "week";
  }
  if (value == 4) {
    return "year";
  }
  if (value == 5) {
    return "range";
  }
  throw std::invalid_argument("Unsupported report type code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportFormat(jint value) -> std::string {
  if (value == 0) {
    return "markdown";
  }
  if (value == 1) {
    return "latex";
  }
  if (value == 2) {
    return "typst";
  }
  throw std::invalid_argument("Unsupported report format code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseCoreResponse(const char* response_json,
                                     std::string_view context)
    -> tt_transport::ResponseEnvelope {
  const auto parsed = tt_transport::ParseResponseEnvelope(
      response_json != nullptr ? std::string_view(response_json)
                               : std::string_view{},
      context);
  if (parsed.HasError()) {
    throw std::runtime_error(parsed.error.message);
  }
  return parsed.envelope;
}

}  // namespace tracer_core::api::android::bridge_internal

#pragma once

#include <jni.h>

#include <array>
#include <exception>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "api/core_c/time_tracer_core_c_api.h"
#include "tracer/transport/envelope.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace time_tracer::api::android::bridge_internal {

namespace tt_transport = tracer::transport;

struct RuntimeHolder {
  TtCoreRuntimeHandle* core_runtime = nullptr;
};

extern std::mutex g_runtime_mutex;
extern RuntimeHolder g_runtime;

constexpr jint kUnsetInt = -1;
constexpr jint kReportModeSingle = 0;
constexpr jint kReportModePeriodBatch = 1;

[[nodiscard]] auto ToUtf8(JNIEnv* env, jstring text) -> std::string;

[[nodiscard]] auto ToJString(JNIEnv* env, std::string_view text) -> jstring;

[[nodiscard]] auto BuildResponseJson(bool ok, std::string_view error_message,
                                     std::string_view content) -> std::string;

auto DestroyRuntimeLocked() -> void;

[[nodiscard]] auto ReadOptionalText(JNIEnv* env, jstring text)
    -> std::optional<std::string>;

[[nodiscard]] auto ToIntVector(JNIEnv* env, jintArray values)
    -> std::vector<int>;

[[nodiscard]] auto ParseDateCheckMode(jint value) -> std::string;

[[nodiscard]] auto ParseDataQueryAction(jint value) -> std::string;

[[nodiscard]] auto ParseReportType(jint value) -> std::string;

[[nodiscard]] auto ParseReportFormat(jint value) -> std::string;

[[nodiscard]] auto ParseCoreResponse(const char* response_json,
                                     std::string_view context)
    -> tt_transport::ResponseEnvelope;

template <typename Fn>
auto ExecuteJniMethod(JNIEnv* env, Fn&& fn) -> jstring {
  try {
    return ToJString(env, fn());
  } catch (const std::exception& exception) {
    return ToJString(env,
                     BuildResponseJson(false, exception.what(), std::string{}));
  } catch (...) {
    return ToJString(
        env, BuildResponseJson(false, "Unknown non-standard C++ exception.",
                               std::string{}));
  }
}

auto NativeInit(JNIEnv* env, jobject thiz, jstring db_path, jstring output_root,
                jstring converter_config_toml_path) -> jstring;

auto NativeIngest(JNIEnv* env, jobject thiz, jstring input_path,
                  jint date_check_mode, jboolean save_processed_output)
    -> jstring;

auto NativeIngestSingleTxtReplaceMonth(JNIEnv* env, jobject thiz,
                                       jstring input_path, jint date_check_mode,
                                       jboolean save_processed_output)
    -> jstring;

auto NativeValidateStructure(JNIEnv* env, jobject thiz, jstring input_path)
    -> jstring;

auto NativeValidateLogic(JNIEnv* env, jobject thiz, jstring input_path,
                         jint date_check_mode) -> jstring;

auto NativeQuery(JNIEnv* env, jobject thiz, jint action, jint year, jint month,
                 jstring from_date, jstring to_date, jstring remark,
                 jstring day_remark, jstring project, jstring root,
                 jint exercise, jint status, jboolean overnight,
                 jboolean reverse, jint limit, jint top_n, jint lookback_days,
                 jboolean score_by_duration, jstring tree_period,
                 jstring tree_period_argument, jint tree_max_depth,
                 jstring output_mode) -> jstring;

auto NativeReport(JNIEnv* env, jobject thiz, jint mode, jint report_type,
                  jstring argument, jint format, jintArray days_list)
    -> jstring;

extern const std::array<JNINativeMethod, 7> kNativeMethods;

auto TryRegisterNativeMethods(JNIEnv* env, const char* class_name) -> bool;

}  // namespace time_tracer::api::android::bridge_internal

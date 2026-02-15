#include <jni.h>

#include <array>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "domain/types/date_check_mode.hpp"
#include "infrastructure/bootstrap/android_runtime_factory.hpp"
#include "nlohmann/json.hpp"

namespace {

using ::ITimeTracerCoreApi;
using nlohmann::json;
namespace core_dto = time_tracer::core::dto;

struct RuntimeHolder {
  std::shared_ptr<ITimeTracerCoreApi> core_api;
  std::shared_ptr<void> runtime_state;
};

std::mutex g_runtime_mutex;
RuntimeHolder g_runtime;

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
  json response = {
      {"ok", ok},
      {"error_message", std::string(error_message)},
      {"content", std::string(content)},
  };
  return response.dump();
}

[[nodiscard]] auto BuildResponseJson(const core_dto::OperationAck& ack,
                                     std::string_view success_content = "")
    -> std::string {
  return BuildResponseJson(ack.ok, ack.error_message,
                           ack.ok ? success_content : "");
}

[[nodiscard]] auto BuildResponseJson(const core_dto::TextOutput& output)
    -> std::string {
  return BuildResponseJson(output.ok, output.error_message, output.content);
}

constexpr jint kUnsetInt = -1;
constexpr jint kReportModeSingle = 0;
constexpr jint kReportModePeriodBatch = 1;

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

[[nodiscard]] auto ParseDateCheckMode(jint value) -> DateCheckMode {
  if (value == 0) {
    return DateCheckMode::kNone;
  }
  if (value == 1) {
    return DateCheckMode::kContinuity;
  }
  if (value == 2) {
    return DateCheckMode::kFull;
  }
  throw std::invalid_argument("Unsupported date_check_mode code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseDataQueryAction(jint value)
    -> core_dto::DataQueryAction {
  if (value == 0) {
    return core_dto::DataQueryAction::kYears;
  }
  if (value == 1) {
    return core_dto::DataQueryAction::kMonths;
  }
  if (value == 2) {
    return core_dto::DataQueryAction::kDays;
  }
  if (value == 3) {
    return core_dto::DataQueryAction::kDaysDuration;
  }
  if (value == 4) {
    return core_dto::DataQueryAction::kDaysStats;
  }
  if (value == 5) {
    return core_dto::DataQueryAction::kSearch;
  }
  if (value == 6) {
    return core_dto::DataQueryAction::kActivitySuggest;
  }
  if (value == 7) {
    return core_dto::DataQueryAction::kTree;
  }
  throw std::invalid_argument("Unsupported query action code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportType(jint value) -> core_dto::ReportQueryType {
  if (value == 0) {
    return core_dto::ReportQueryType::kDay;
  }
  if (value == 1) {
    return core_dto::ReportQueryType::kMonth;
  }
  if (value == 2) {
    return core_dto::ReportQueryType::kRecent;
  }
  if (value == 3) {
    return core_dto::ReportQueryType::kWeek;
  }
  if (value == 4) {
    return core_dto::ReportQueryType::kYear;
  }
  if (value == 5) {
    return core_dto::ReportQueryType::kRange;
  }
  throw std::invalid_argument("Unsupported report type code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportFormat(jint value) -> ReportFormat {
  if (value == 0) {
    return ReportFormat::kMarkdown;
  }
  if (value == 1) {
    return ReportFormat::kLaTeX;
  }
  if (value == 2) {
    return ReportFormat::kTyp;
  }
  throw std::invalid_argument("Unsupported report format code: " +
                              std::to_string(value));
}

[[nodiscard]] auto GetInitializedRuntime() -> RuntimeHolder {
  std::scoped_lock lock(g_runtime_mutex);
  return g_runtime;
}

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

auto NativeInit(JNIEnv* env, jobject /*thiz*/, jstring db_path,
                jstring output_root, jstring converter_config_toml_path)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string db_path_utf8 = ToUtf8(env, db_path);
    const std::string output_root_utf8 = ToUtf8(env, output_root);
    const std::string converter_config_toml_path_utf8 =
        ToUtf8(env, converter_config_toml_path);
    if (output_root_utf8.empty()) {
      return BuildResponseJson(false, "outputRoot must not be empty.",
                               std::string{});
    }
    if (converter_config_toml_path_utf8.empty()) {
      return BuildResponseJson(
          false, "converterConfigTomlPath must not be empty.", std::string{});
    }

    infrastructure::bootstrap::AndroidRuntimeRequest request;
    request.db_path = db_path_utf8;
    request.output_root = output_root_utf8;
    request.converter_config_toml_path = converter_config_toml_path_utf8;

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      return BuildResponseJson(false, "Failed to initialize core runtime.",
                               std::string{});
    }

    std::scoped_lock lock(g_runtime_mutex);
    g_runtime.core_api = std::move(runtime.core_api);
    g_runtime.runtime_state = std::move(runtime.runtime_state);

    return BuildResponseJson(true, std::string{}, "initialized");
  });
}

auto NativeIngest(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                  jint date_check_mode, jboolean save_processed_output)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const RuntimeHolder runtime = GetInitializedRuntime();
    if (!runtime.core_api) {
      return BuildResponseJson(false, "nativeInit must be called first.",
                               std::string{});
    }

    const std::string input_path_utf8 = ToUtf8(env, input_path);
    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }

    core_dto::IngestRequest request;
    request.input_path = input_path_utf8;
    request.date_check_mode = ParseDateCheckMode(date_check_mode);
    request.save_processed_output = save_processed_output == JNI_TRUE;

    const auto ack = runtime.core_api->RunIngest(request);
    return BuildResponseJson(ack, "ingest completed");
  });
}

auto NativeQuery(JNIEnv* env, jobject /*thiz*/, jint action, jint year,
                 jint month, jstring from_date, jstring to_date, jstring remark,
                 jstring day_remark, jstring project, jint exercise,
                 jint status, jboolean overnight, jboolean reverse, jint limit,
                 jint top_n, jint lookback_days, jboolean score_by_duration,
                 jstring tree_period, jstring tree_period_argument,
                 jint tree_max_depth) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const RuntimeHolder runtime = GetInitializedRuntime();
    if (!runtime.core_api) {
      return BuildResponseJson(false, "nativeInit must be called first.",
                               std::string{});
    }

    core_dto::DataQueryRequest request;
    request.action = ParseDataQueryAction(action);
    if (year > 0) {
      request.year = static_cast<int>(year);
    }
    if (month > 0) {
      request.month = static_cast<int>(month);
    }
    request.from_date = ReadOptionalText(env, from_date);
    request.to_date = ReadOptionalText(env, to_date);
    request.remark = ReadOptionalText(env, remark);
    request.day_remark = ReadOptionalText(env, day_remark);
    request.project = ReadOptionalText(env, project);
    if (exercise != kUnsetInt) {
      request.exercise = static_cast<int>(exercise);
    }
    if (status != kUnsetInt) {
      request.status = static_cast<int>(status);
    }
    request.overnight = overnight == JNI_TRUE;
    request.reverse = reverse == JNI_TRUE;
    if (limit != kUnsetInt) {
      request.limit = static_cast<int>(limit);
    }
    if (top_n != kUnsetInt) {
      request.top_n = static_cast<int>(top_n);
    }
    if (lookback_days != kUnsetInt) {
      request.lookback_days = static_cast<int>(lookback_days);
    }
    request.activity_score_by_duration = score_by_duration == JNI_TRUE;
    request.tree_period = ReadOptionalText(env, tree_period);
    request.tree_period_argument = ReadOptionalText(env, tree_period_argument);
    if (tree_max_depth != kUnsetInt) {
      request.tree_max_depth = static_cast<int>(tree_max_depth);
    }

    const auto output = runtime.core_api->RunDataQuery(request);
    return BuildResponseJson(output);
  });
}

auto NativeReport(JNIEnv* env, jobject /*thiz*/, jint mode, jint report_type,
                  jstring argument, jint format, jintArray days_list)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const RuntimeHolder runtime = GetInitializedRuntime();
    if (!runtime.core_api) {
      return BuildResponseJson(false, "nativeInit must be called first.",
                               std::string{});
    }

    const ReportFormat request_format = ParseReportFormat(format);
    if (mode == kReportModePeriodBatch) {
      core_dto::PeriodBatchQueryRequest request;
      request.days_list = ToIntVector(env, days_list);
      request.format = request_format;
      const auto output = runtime.core_api->RunPeriodBatchQuery(request);
      return BuildResponseJson(output);
    }

    if (mode != kReportModeSingle) {
      throw std::invalid_argument("Unsupported report mode code: " +
                                  std::to_string(mode));
    }

    core_dto::ReportQueryRequest request;
    request.type = ParseReportType(report_type);
    request.argument = ToUtf8(env, argument);
    request.format = request_format;

    const auto output = runtime.core_api->RunReportQuery(request);
    return BuildResponseJson(output);
  });
}

const std::array<JNINativeMethod, 4> kNativeMethods = {
    JNINativeMethod{
        const_cast<char*>("nativeInit"),
        const_cast<char*>(
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)"
            "Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeInit),
    },
    JNINativeMethod{
        const_cast<char*>("nativeIngest"),
        const_cast<char*>("(Ljava/lang/String;IZ)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeIngest),
    },
    JNINativeMethod{
        const_cast<char*>("nativeQuery"),
        const_cast<char*>(
            "(IIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;Ljava/lang/String;IIZZIIIZLjava/lang/String;"
            "Ljava/lang/String;I)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeQuery),
    },
    JNINativeMethod{
        const_cast<char*>("nativeReport"),
        const_cast<char*>("(IILjava/lang/String;I[I)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeReport),
    },
};

auto TryRegisterNativeMethods(JNIEnv* env, const char* class_name) -> bool {
  jclass bridge_class = env->FindClass(class_name);
  if (bridge_class == nullptr) {
    env->ExceptionClear();
    return false;
  }

  const jint result =
      env->RegisterNatives(bridge_class, kNativeMethods.data(),
                           static_cast<jint>(kNativeMethods.size()));
  env->DeleteLocalRef(bridge_class);
  if (result != JNI_OK) {
    env->ExceptionClear();
    return false;
  }
  return true;
}

}  // namespace

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
  JNIEnv* env = nullptr;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ||
      env == nullptr) {
    return JNI_ERR;
  }

  constexpr std::array<const char*, 4> kBridgeClassCandidates = {
      "com/time_tracer/core/NativeBridge",
      "com/timetracer/NativeBridge",
      "com/example/tracer/NativeBridge",
      "NativeBridge",
  };

  bool registered = false;
  for (const char* class_name : kBridgeClassCandidates) {
    if (TryRegisterNativeMethods(env, class_name)) {
      registered = true;
      break;
    }
  }

  return registered ? JNI_VERSION_1_6 : JNI_ERR;
}

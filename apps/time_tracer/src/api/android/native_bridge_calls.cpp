#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>

#include "api/android/native_bridge_internal.hpp"
#include "domain/ports/diagnostics.hpp"

namespace time_tracer::api::android::bridge_internal {

using nlohmann::json;

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

    const char* db_path_arg =
        db_path_utf8.empty() ? nullptr : db_path_utf8.c_str();
    TtCoreRuntimeHandle* created_runtime =
        tracer_core_runtime_create(db_path_arg, output_root_utf8.c_str(),
                                   converter_config_toml_path_utf8.c_str());
    if (created_runtime == nullptr) {
      const char* error_message = tracer_core_last_error();
      const std::string details =
          (error_message != nullptr && error_message[0] != '\0')
              ? std::string(error_message)
              : std::string("Failed to initialize core runtime.");
      return BuildResponseJson(false, details, std::string{});
    }

    {
      std::scoped_lock lock(g_runtime_mutex);
      DestroyRuntimeLocked();
      g_runtime.core_runtime = created_runtime;
    }

    const std::string kErrorLogPath =
        time_tracer::domain::ports::GetCurrentRunErrorLogPath();
    const std::string kContentJson = json{
        {"status", "initialized"},
        {"error_log_path",
         kErrorLogPath}}.dump();
    return BuildResponseJson(true, std::string{}, kContentJson);
  });
}

auto NativeIngest(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                  jint date_check_mode, jboolean save_processed_output)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }

    tt_transport::IngestRequestPayload request_payload{};
    request_payload.input_path = input_path_utf8;
    request_payload.date_check_mode = ParseDateCheckMode(date_check_mode);
    request_payload.save_processed_output = (save_processed_output == JNI_TRUE);
    const std::string request_json =
        tt_transport::EncodeIngestRequest(request_payload);

    tt_transport::ResponseEnvelope response_payload{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      response_payload =
          ParseCoreResponse(tracer_core_runtime_ingest_json(
                                g_runtime.core_runtime, request_json.c_str()),
                            "nativeIngest");
    }

    if (response_payload.ok) {
      response_payload.content = "ingest completed";
    }
    return tt_transport::SerializeResponseEnvelope(response_payload);
  });
}

auto NativeIngestSingleTxtReplaceMonth(JNIEnv* env, jobject /*thiz*/,
                                       jstring input_path, jint date_check_mode,
                                       jboolean save_processed_output)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }

    tt_transport::IngestRequestPayload request_payload{};
    request_payload.input_path = input_path_utf8;
    request_payload.date_check_mode = ParseDateCheckMode(date_check_mode);
    request_payload.save_processed_output = (save_processed_output == JNI_TRUE);
    request_payload.ingest_mode = std::string("single_txt_replace_month");
    const std::string request_json =
        tt_transport::EncodeIngestRequest(request_payload);

    tt_transport::ResponseEnvelope response_payload{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      response_payload =
          ParseCoreResponse(tracer_core_runtime_ingest_json(
                                g_runtime.core_runtime, request_json.c_str()),
                            "nativeIngest(single_txt_replace_month)");
    }

    if (response_payload.ok) {
      response_payload.content = "ingest(single_txt_replace_month) completed";
    }
    return tt_transport::SerializeResponseEnvelope(response_payload);
  });
}

auto NativeValidateStructure(JNIEnv* env, jobject /*thiz*/, jstring input_path)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }

    const json request_payload = {
        {"input_path", input_path_utf8},
    };
    const std::string request_json = request_payload.dump();

    tt_transport::ResponseEnvelope response_payload{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      response_payload =
          ParseCoreResponse(tracer_core_runtime_validate_structure_json(
                                g_runtime.core_runtime, request_json.c_str()),
                            "nativeValidateStructure");
    }

    if (response_payload.ok) {
      response_payload.content = "validate structure completed";
    }
    return tt_transport::SerializeResponseEnvelope(response_payload);
  });
}

auto NativeValidateLogic(JNIEnv* env, jobject /*thiz*/, jstring input_path,
                         jint date_check_mode) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string input_path_utf8 = ToUtf8(env, input_path);
    if (input_path_utf8.empty()) {
      return BuildResponseJson(false, "inputPath must not be empty.",
                               std::string{});
    }

    const json request_payload = {
        {"input_path", input_path_utf8},
        {"date_check_mode", ParseDateCheckMode(date_check_mode)},
    };
    const std::string request_json = request_payload.dump();

    tt_transport::ResponseEnvelope response_payload{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      response_payload =
          ParseCoreResponse(tracer_core_runtime_validate_logic_json(
                                g_runtime.core_runtime, request_json.c_str()),
                            "nativeValidateLogic");
    }

    if (response_payload.ok) {
      response_payload.content = "validate logic completed";
    }
    return tt_transport::SerializeResponseEnvelope(response_payload);
  });
}

auto NativeQuery(JNIEnv* env, jobject /*thiz*/, jint action, jint year,
                 jint month, jstring from_date, jstring to_date, jstring remark,
                 jstring day_remark, jstring project, jstring root,
                 jint exercise, jint status, jboolean overnight,
                 jboolean reverse, jint limit, jint top_n, jint lookback_days,
                 jboolean score_by_duration, jstring tree_period,
                 jstring tree_period_argument, jint tree_max_depth,
                 jstring output_mode) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    tt_transport::QueryRequestPayload request_payload{};
    request_payload.action = ParseDataQueryAction(action);
    request_payload.overnight = (overnight == JNI_TRUE);
    request_payload.reverse = (reverse == JNI_TRUE);
    request_payload.activity_score_by_duration =
        (score_by_duration == JNI_TRUE);

    if (year > 0) {
      request_payload.year = static_cast<int>(year);
    }
    if (month > 0) {
      request_payload.month = static_cast<int>(month);
    }
    if (const auto from_date_value = ReadOptionalText(env, from_date);
        from_date_value.has_value()) {
      request_payload.from_date = *from_date_value;
    }
    if (const auto to_date_value = ReadOptionalText(env, to_date);
        to_date_value.has_value()) {
      request_payload.to_date = *to_date_value;
    }
    if (const auto remark_value = ReadOptionalText(env, remark);
        remark_value.has_value()) {
      request_payload.remark = *remark_value;
    }
    if (const auto day_remark_value = ReadOptionalText(env, day_remark);
        day_remark_value.has_value()) {
      request_payload.day_remark = *day_remark_value;
    }
    if (const auto project_value = ReadOptionalText(env, project);
        project_value.has_value()) {
      request_payload.project = *project_value;
    }
    if (const auto root_value = ReadOptionalText(env, root);
        root_value.has_value()) {
      request_payload.root = *root_value;
    }
    if (exercise != kUnsetInt) {
      request_payload.exercise = static_cast<int>(exercise);
    }
    if (status != kUnsetInt) {
      request_payload.status = static_cast<int>(status);
    }
    if (limit != kUnsetInt) {
      request_payload.limit = static_cast<int>(limit);
    }
    if (top_n != kUnsetInt) {
      request_payload.top_n = static_cast<int>(top_n);
    }
    if (lookback_days != kUnsetInt) {
      request_payload.lookback_days = static_cast<int>(lookback_days);
    }
    if (const auto tree_period_value = ReadOptionalText(env, tree_period);
        tree_period_value.has_value()) {
      request_payload.tree_period = *tree_period_value;
    }
    if (const auto tree_period_argument_value =
            ReadOptionalText(env, tree_period_argument);
        tree_period_argument_value.has_value()) {
      request_payload.tree_period_argument = *tree_period_argument_value;
    }
    if (tree_max_depth != kUnsetInt) {
      request_payload.tree_max_depth = static_cast<int>(tree_max_depth);
    }
    if (const auto output_mode_value = ReadOptionalText(env, output_mode);
        output_mode_value.has_value()) {
      request_payload.output_mode = *output_mode_value;
    }

    const std::string request_json =
        tt_transport::EncodeQueryRequest(request_payload);
    tt_transport::ResponseEnvelope response_payload{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      response_payload =
          ParseCoreResponse(tracer_core_runtime_query_json(
                                g_runtime.core_runtime, request_json.c_str()),
                            "nativeQuery");
    }
    return tt_transport::SerializeResponseEnvelope(response_payload);
  });
}

auto NativeReport(JNIEnv* env, jobject /*thiz*/, jint mode, jint report_type,
                  jstring argument, jint format, jintArray days_list)
    -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    const std::string request_format = ParseReportFormat(format);

    if (mode == kReportModePeriodBatch) {
      tt_transport::ReportBatchRequestPayload request_payload{};
      request_payload.days_list = ToIntVector(env, days_list);
      request_payload.format = request_format;
      const std::string request_json =
          tt_transport::EncodeReportBatchRequest(request_payload);

      tt_transport::ResponseEnvelope response_payload{};
      {
        std::scoped_lock lock(g_runtime_mutex);
        if (g_runtime.core_runtime == nullptr) {
          return BuildResponseJson(false, "nativeInit must be called first.",
                                   std::string{});
        }
        response_payload =
            ParseCoreResponse(tracer_core_runtime_report_batch_json(
                                  g_runtime.core_runtime, request_json.c_str()),
                              "nativeReport(batch)");
      }
      return tt_transport::SerializeResponseEnvelope(response_payload);
    }

    if (mode != kReportModeSingle) {
      throw std::invalid_argument("Unsupported report mode code: " +
                                  std::to_string(mode));
    }

    tt_transport::ReportRequestPayload request_payload{};
    request_payload.type = ParseReportType(report_type);
    request_payload.argument = ToUtf8(env, argument);
    request_payload.format = request_format;
    const std::string request_json =
        tt_transport::EncodeReportRequest(request_payload);

    tt_transport::ResponseEnvelope response_payload{};
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      response_payload =
          ParseCoreResponse(tracer_core_runtime_report_json(
                                g_runtime.core_runtime, request_json.c_str()),
                            "nativeReport(single)");
    }
    return tt_transport::SerializeResponseEnvelope(response_payload);
  });
}

}  // namespace time_tracer::api::android::bridge_internal

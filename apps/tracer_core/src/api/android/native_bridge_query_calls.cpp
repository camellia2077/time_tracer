// api/android/native_bridge_query_calls.cpp
#include <stdexcept>
#include <string>

#include "api/android/native_bridge_internal.hpp"

namespace tracer_core::api::android::bridge_internal {

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

auto NativeTree(JNIEnv* env, jobject /*thiz*/, jboolean list_roots,
                jstring root_pattern, jint max_depth, jstring period,
                jstring period_argument, jstring root) -> jstring {
  return ExecuteJniMethod(env, [&]() -> std::string {
    tt_transport::TreeRequestPayload request_payload{};
    request_payload.list_roots = (list_roots == JNI_TRUE);
    if (const auto root_pattern_value = ReadOptionalText(env, root_pattern);
        root_pattern_value.has_value()) {
      request_payload.root_pattern = *root_pattern_value;
    }
    if (max_depth != kUnsetInt) {
      request_payload.max_depth = static_cast<int>(max_depth);
    }
    if (const auto period_value = ReadOptionalText(env, period);
        period_value.has_value()) {
      request_payload.period = *period_value;
    }
    if (const auto period_argument_value =
            ReadOptionalText(env, period_argument);
        period_argument_value.has_value()) {
      request_payload.period_argument = *period_argument_value;
    }
    if (const auto root_value = ReadOptionalText(env, root);
        root_value.has_value()) {
      request_payload.root = *root_value;
    }

    const std::string request_json =
        tt_transport::EncodeTreeRequest(request_payload);
    std::string raw_response;
    {
      std::scoped_lock lock(g_runtime_mutex);
      if (g_runtime.core_runtime == nullptr) {
        return BuildResponseJson(false, "nativeInit must be called first.",
                                 std::string{});
      }
      const char* response = tracer_core_runtime_tree_json(
          g_runtime.core_runtime, request_json.c_str());
      if (response == nullptr || response[0] == '\0') {
        throw std::runtime_error("nativeTree received empty core response.");
      }
      raw_response = response;
    }

    const auto tree_payload = tt_transport::DecodeTreeResponse(raw_response);
    if (!tree_payload.ok) {
      return BuildResponseJson(false,
                               tree_payload.error_message.empty()
                                   ? "tree query failed."
                                   : tree_payload.error_message,
                               std::string{});
    }
    return BuildResponseJson(true, std::string{},
                             tt_transport::EncodeTreeResponse(tree_payload));
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

}  // namespace tracer_core::api::android::bridge_internal

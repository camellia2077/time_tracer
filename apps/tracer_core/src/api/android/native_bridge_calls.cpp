// api/android/native_bridge_calls.cpp
#include <nlohmann/json.hpp>
#include <string>

#include "api/android/native_bridge_internal.hpp"
#include "domain/ports/diagnostics.hpp"

namespace tracer_core::api::android::bridge_internal {

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
        tracer_core::domain::ports::GetCurrentRunErrorLogPath();
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

}  // namespace tracer_core::api::android::bridge_internal

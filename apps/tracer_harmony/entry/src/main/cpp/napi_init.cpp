#include "napi/native_api.h"
#include <hilog/log.h>
#include <string>
#include <vector>

// 引入 Core C ABI (确保 CMakeLists.txt 中包含了该头文件所在的路径)
#include "api/core_c/tracer_core_c_api.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200
#define LOG_TAG "TracerNAPI"

// Helper: 获取字符串参数
static std::string GetStringArgument(napi_env env, napi_value value) {
  size_t length = 0;
  napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
  if (status != napi_ok || length == 0) return "";
  std::vector<char> buffer(length + 1);
  napi_get_value_string_utf8(env, value, buffer.data(), length + 1, &length);
  return std::string(buffer.data());
}

// 包装 TtCoreRuntimeHandle* 以便在 ArkTS 侧持有引用
static void DestructRuntimeHandle(napi_env env, void* nativeObject, void* finalize_hint) {
    if (nativeObject != nullptr) {
        // TODO: Ensure tracer_core_runtime_destroy is linked
        // tracer_core_runtime_destroy(static_cast<TtCoreRuntimeHandle*>(nativeObject));
    }
}

// args[0]: dbPath (string)
// args[1]: outputRoot (string)
// args[2]: converterConfigTomlPath (string)
static napi_value CreateRuntime(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 3) {
        napi_throw_error(env, nullptr, "Requires 3 arguments: dbPath, outputRoot, configPath");
        return nullptr;
    }

    std::string dbPath = GetStringArgument(env, args[0]);
    std::string outputRoot = GetStringArgument(env, args[1]);
    std::string configPath = GetStringArgument(env, args[2]);

    OH_LOG_INFO(LOG_APP, "CreateRuntime called: %{public}s, %{public}s", dbPath.c_str(), outputRoot.c_str());

    // NOTE: This requires time_tracer_core.dll / .so to be packaged and linked properly.
    // TtCoreRuntimeHandle* handle = tracer_core_runtime_create(
    //     dbPath.c_str(), 
    //     outputRoot.c_str(), 
    //     configPath.c_str()
    // );

    // Mock Handle for now until Native prebuilt .so is ready
    void* mockHandle = reinterpret_cast<void*>(0x12345678);

    napi_value externalHandle;
    napi_create_external(env, mockHandle, DestructRuntimeHandle, nullptr, &externalHandle);
    return externalHandle;
}

// args[0]: external Handle
// args[1]: request json
static napi_value IngestJson(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Requires 2 arguments: handle, requestJson");
        return nullptr;
    }

    void* handlePtr = nullptr;
    napi_get_value_external(env, args[0], &handlePtr);
    if (!handlePtr) {
        napi_throw_error(env, nullptr, "Invalid handle");
        return nullptr;
    }

    std::string requestJson = GetStringArgument(env, args[1]);
    OH_LOG_INFO(LOG_APP, "IngestJson called with payload length: %{public}zu", requestJson.length());

    // Mock direct success response
    const char* mockResponse = "{\"ok\":true,\"error_message\":\"\",\"content\":\"mock_ingest_ok\"}";

    napi_value result;
    napi_create_string_utf8(env, mockResponse, NAPI_AUTO_LENGTH, &result);
    return result;
}

// args[0]: external Handle
// args[1]: request json
static napi_value TreeJson(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Requires 2 arguments: handle, requestJson");
        return nullptr;
    }

    void* handlePtr = nullptr;
    napi_get_value_external(env, args[0], &handlePtr);

    std::string requestJson = GetStringArgument(env, args[1]);
    OH_LOG_INFO(LOG_APP, "TreeJson called");
    
    // Mock response demonstrating month query
    const char* mockResponse = R"({
      "ok": true,
      "found": true,
      "error_message": "",
      "nodes": [
        {
          "name": "work",
          "duration_seconds": 36000,
          "children": [
             {"name": "meeting", "duration_seconds": 7200}
          ]
        },
        {
          "name": "play",
          "duration_seconds": 18000
        }
      ]
    })";

    napi_value result;
    napi_create_string_utf8(env, mockResponse, NAPI_AUTO_LENGTH, &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"createRuntime", nullptr, CreateRuntime, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"ingestJson", nullptr, IngestJson, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"treeJson", nullptr, TreeJson, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module tracerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) {
    napi_module_register(&tracerModule);
}

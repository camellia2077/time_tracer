// host/native_bridge_progress.cpp
#include <array>
#include <string>

#include "host/exchange/crypto_progress_bridge.hpp"
#include "host/native_bridge_progress.hpp"
#include "api/android_jni/native_bridge_internal.hpp"
#include "application/dto/exchange_requests.hpp"

namespace tracer_core::api::android::bridge_internal {

namespace app_dto = tracer_core::core::dto;

namespace {

auto EmitCryptoProgress(JNIEnv* env,
                        const app_dto::TracerExchangeProgressSnapshot& snapshot)
    -> void {
  if (env == nullptr) {
    return;
  }

  constexpr std::array<const char*, 4> kBridgeClassCandidates = {
      "com/example/tracer/NativeBridge",
      "com/time_tracer/core/NativeBridge",
      "com/timetracer/NativeBridge",
      "NativeBridge",
  };

  jclass bridge_class = nullptr;
  for (const char* class_name : kBridgeClassCandidates) {
    bridge_class = env->FindClass(class_name);
    if (bridge_class != nullptr) {
      break;
    }
    env->ExceptionClear();
  }
  if (bridge_class == nullptr) {
    return;
  }

  jmethodID on_progress = env->GetStaticMethodID(
      bridge_class, "onCryptoProgressJson", "(Ljava/lang/String;)V");
  if (on_progress == nullptr) {
    env->ExceptionClear();
    env->DeleteLocalRef(bridge_class);
    return;
  }

  const std::string progress_json =
      tracer_core::shell::crypto_progress_bridge::BuildProgressSnapshotJson(
          snapshot);

  jstring payload_jstring = ToJString(env, progress_json);
  env->CallStaticVoidMethod(bridge_class, on_progress, payload_jstring);
  if (env->ExceptionCheck()) {
    env->ExceptionClear();
  }
  env->DeleteLocalRef(payload_jstring);
  env->DeleteLocalRef(bridge_class);
}

}  // namespace

auto BuildTracerExchangeProgressObserver(JNIEnv* env)
    -> app_dto::TracerExchangeProgressObserver {
  return [env](const app_dto::TracerExchangeProgressSnapshot& snapshot) {
    EmitCryptoProgress(env, snapshot);
    return app_dto::TracerExchangeProgressControl::kContinue;
  };
}

}  // namespace tracer_core::api::android::bridge_internal

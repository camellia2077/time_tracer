// api/android/native_bridge_registration.cpp
#include <array>
#include <mutex>

#include "api/android/native_bridge_internal.hpp"

namespace tracer_core::api::android::bridge_internal {

const std::array<JNINativeMethod, 10> kNativeMethods = {
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
        const_cast<char*>("nativeIngestSingleTxtReplaceMonth"),
        const_cast<char*>("(Ljava/lang/String;IZ)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeIngestSingleTxtReplaceMonth),
    },
    JNINativeMethod{
        const_cast<char*>("nativeValidateStructure"),
        const_cast<char*>("(Ljava/lang/String;)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeValidateStructure),
    },
    JNINativeMethod{
        const_cast<char*>("nativeValidateLogic"),
        const_cast<char*>("(Ljava/lang/String;I)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeValidateLogic),
    },
    JNINativeMethod{
        const_cast<char*>("nativeEncryptFile"),
        const_cast<char*>(
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;)"
            "Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeEncryptFile),
    },
    JNINativeMethod{
        const_cast<char*>("nativeDecryptFile"),
        const_cast<char*>(
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)"
            "Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeDecryptFile),
    },
    JNINativeMethod{
        const_cast<char*>("nativeQuery"),
        const_cast<char*>(
            "(IIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIZZIIIZ"
            "Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)"
            "Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeQuery),
    },
    JNINativeMethod{
        const_cast<char*>("nativeTree"),
        const_cast<char*>(
            "(ZLjava/lang/String;ILjava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;)Ljava/lang/String;"),
        reinterpret_cast<void*>(&NativeTree),
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

}  // namespace tracer_core::api::android::bridge_internal

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
    if (tracer_core::api::android::bridge_internal::TryRegisterNativeMethods(
            env, class_name)) {
      registered = true;
      break;
    }
  }

  return registered ? JNI_VERSION_1_6 : JNI_ERR;
}

extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* /*vm*/,
                                               void* /*reserved*/) {
  std::scoped_lock lock(
      tracer_core::api::android::bridge_internal::g_runtime_mutex);
  tracer_core::api::android::bridge_internal::DestroyRuntimeLocked();
}

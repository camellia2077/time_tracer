#ifndef API_ANDROID_JNI_RUNTIME_CODE_BRIDGE_H_
#define API_ANDROID_JNI_RUNTIME_CODE_BRIDGE_H_

#include <string>

namespace tracer_core::shell::jni_bridge {

[[nodiscard]] auto ParseDateCheckModeCode(int value) -> std::string;
[[nodiscard]] auto ParseRecordTimeOrderModeCode(int value) -> std::string;
[[nodiscard]] auto ParseDataQueryActionCode(int value) -> std::string;
[[nodiscard]] auto ParseInsightsTypeCode(int value) -> std::string;
[[nodiscard]] auto ParseInsightsFormatCode(int value) -> std::string;

}  // namespace tracer_core::shell::jni_bridge

#endif  // API_ANDROID_JNI_RUNTIME_CODE_BRIDGE_H_

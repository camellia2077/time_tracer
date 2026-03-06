#ifndef TRACER_CORE_BRIDGE_COMMON_JNI_BRIDGE_UTILS_HPP_
#define TRACER_CORE_BRIDGE_COMMON_JNI_BRIDGE_UTILS_HPP_

#include <string>
#include <string_view>

#include "tracer/transport/envelope.hpp"

namespace tracer_core_bridge_common::jni {

[[nodiscard]] auto BuildResponseJson(bool ok, std::string_view error_message,
                                     std::string_view content) -> std::string;

[[nodiscard]] auto ParseDateCheckModeCode(int value) -> std::string;
[[nodiscard]] auto ParseDataQueryActionCode(int value) -> std::string;
[[nodiscard]] auto ParseReportTypeCode(int value) -> std::string;
[[nodiscard]] auto ParseReportFormatCode(int value) -> std::string;

[[nodiscard]] auto ParseCoreResponse(const char* response_json,
                                     std::string_view context)
    -> tracer::transport::ResponseEnvelope;

}  // namespace tracer_core_bridge_common::jni

#endif  // TRACER_CORE_BRIDGE_COMMON_JNI_BRIDGE_UTILS_HPP_

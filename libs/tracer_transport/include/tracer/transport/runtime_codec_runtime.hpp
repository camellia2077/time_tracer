#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto DecodeAckResponse(std::string_view response_json,
                                     std::string_view context = {})
    -> AckResponsePayload;
[[nodiscard]] auto DecodeTextResponse(std::string_view response_json,
                                      std::string_view context = {})
    -> TextResponsePayload;
[[nodiscard]] auto DecodeRuntimeCheckResponse(std::string_view response_json)
    -> RuntimeCheckResponsePayload;
[[nodiscard]] auto DecodeResolveCliContextResponse(
    std::string_view response_json) -> ResolveCliContextResponsePayload;

}  // namespace tracer::transport

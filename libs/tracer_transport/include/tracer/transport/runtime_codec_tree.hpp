#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_requests.hpp"
#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto DecodeTreeRequest(std::string_view request_json)
    -> TreeRequestPayload;
[[nodiscard]] auto EncodeTreeRequest(const TreeRequestPayload& request)
    -> std::string;
[[nodiscard]] auto DecodeTreeResponse(std::string_view response_json)
    -> TreeResponsePayload;
[[nodiscard]] auto EncodeTreeResponse(const TreeResponsePayload& response)
    -> std::string;

}  // namespace tracer::transport

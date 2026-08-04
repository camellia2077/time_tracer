#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_requests.hpp"
#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto DecodeQueryRequest(std::string_view request_json)
    -> QueryRequestPayload;
[[nodiscard]] auto EncodeQueryRequest(const QueryRequestPayload& request)
    -> std::string;
[[nodiscard]] auto EncodeQueryResponse(const QueryResponsePayload& response)
    -> std::string;

}  // namespace tracer::transport

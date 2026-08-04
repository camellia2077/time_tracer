#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_requests.hpp"
#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto DecodeIngestRequest(std::string_view request_json)
    -> IngestRequestPayload;
[[nodiscard]] auto EncodeIngestRequest(const IngestRequestPayload& request)
    -> std::string;
[[nodiscard]] auto EncodeIngestResponse(const IngestResponsePayload& response)
    -> std::string;
[[nodiscard]] auto DecodeIngestSyncStatusRequest(std::string_view request_json)
    -> IngestSyncStatusRequestPayload;
[[nodiscard]] auto EncodeIngestSyncStatusRequest(
    const IngestSyncStatusRequestPayload& request) -> std::string;
[[nodiscard]] auto EncodeIngestSyncStatusResponse(
    const IngestSyncStatusResponsePayload& response) -> std::string;

}  // namespace tracer::transport

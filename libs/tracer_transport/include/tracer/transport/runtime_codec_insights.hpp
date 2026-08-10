#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_requests.hpp"
#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto EncodeInsightsResponse(const InsightsResponsePayload& response)
    -> std::string;
[[nodiscard]] auto DecodeTemporalInsightsRequest(std::string_view request_json)
    -> TemporalInsightsRequestPayload;
[[nodiscard]] auto EncodeTemporalInsightsRequest(
    const TemporalInsightsRequestPayload& request) -> std::string;
[[nodiscard]] auto EncodeInsightsTargetsResponse(
    const InsightsTargetsResponsePayload& response) -> std::string;
[[nodiscard]] auto DecodeInsightsBatchRequest(std::string_view request_json)
    -> InsightsBatchRequestPayload;
[[nodiscard]] auto EncodeInsightsBatchRequest(
    const InsightsBatchRequestPayload& request) -> std::string;
[[nodiscard]] auto EncodeInsightsBatchResponse(
    const InsightsBatchResponsePayload& response) -> std::string;

}  // namespace tracer::transport

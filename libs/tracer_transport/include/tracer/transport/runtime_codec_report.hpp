#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_requests.hpp"
#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto EncodeReportResponse(const ReportResponsePayload& response)
    -> std::string;
[[nodiscard]] auto DecodeTemporalReportRequest(std::string_view request_json)
    -> TemporalReportRequestPayload;
[[nodiscard]] auto EncodeTemporalReportRequest(
    const TemporalReportRequestPayload& request) -> std::string;
[[nodiscard]] auto EncodeReportTargetsResponse(
    const ReportTargetsResponsePayload& response) -> std::string;
[[nodiscard]] auto DecodeReportBatchRequest(std::string_view request_json)
    -> ReportBatchRequestPayload;
[[nodiscard]] auto EncodeReportBatchRequest(
    const ReportBatchRequestPayload& request) -> std::string;
[[nodiscard]] auto EncodeReportBatchResponse(
    const ReportBatchResponsePayload& response) -> std::string;

}  // namespace tracer::transport

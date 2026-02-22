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
[[nodiscard]] auto DecodeRuntimeCheckResponse(std::string_view response_json)
    -> RuntimeCheckResponsePayload;
[[nodiscard]] auto DecodeResolveCliContextResponse(
    std::string_view response_json) -> ResolveCliContextResponsePayload;
[[nodiscard]] auto EncodeCapabilitiesResponse(
    const CapabilitiesResponsePayload& response) -> std::string;

[[nodiscard]] auto DecodeConvertRequest(std::string_view request_json)
    -> ConvertRequestPayload;
[[nodiscard]] auto EncodeConvertRequest(const ConvertRequestPayload& request)
    -> std::string;

[[nodiscard]] auto DecodeImportRequest(std::string_view request_json)
    -> ImportRequestPayload;
[[nodiscard]] auto EncodeImportRequest(const ImportRequestPayload& request)
    -> std::string;

[[nodiscard]] auto DecodeValidateStructureRequest(std::string_view request_json)
    -> ValidateStructureRequestPayload;
[[nodiscard]] auto EncodeValidateStructureRequest(
    const ValidateStructureRequestPayload& request) -> std::string;

[[nodiscard]] auto DecodeValidateLogicRequest(std::string_view request_json)
    -> ValidateLogicRequestPayload;
[[nodiscard]] auto EncodeValidateLogicRequest(
    const ValidateLogicRequestPayload& request) -> std::string;

[[nodiscard]] auto DecodeQueryRequest(std::string_view request_json)
    -> QueryRequestPayload;
[[nodiscard]] auto EncodeQueryRequest(const QueryRequestPayload& request)
    -> std::string;
[[nodiscard]] auto EncodeQueryResponse(const QueryResponsePayload& response)
    -> std::string;

[[nodiscard]] auto DecodeReportRequest(std::string_view request_json)
    -> ReportRequestPayload;
[[nodiscard]] auto EncodeReportRequest(const ReportRequestPayload& request)
    -> std::string;
[[nodiscard]] auto EncodeReportResponse(const ReportResponsePayload& response)
    -> std::string;
[[nodiscard]] auto DecodeReportBatchRequest(std::string_view request_json)
    -> ReportBatchRequestPayload;
[[nodiscard]] auto EncodeReportBatchRequest(
    const ReportBatchRequestPayload& request) -> std::string;
[[nodiscard]] auto EncodeReportBatchResponse(
    const ReportBatchResponsePayload& response) -> std::string;

[[nodiscard]] auto DecodeExportRequest(std::string_view request_json)
    -> ExportRequestPayload;
[[nodiscard]] auto EncodeExportRequest(const ExportRequestPayload& request)
    -> std::string;
[[nodiscard]] auto EncodeExportResponse(const ExportResponsePayload& response)
    -> std::string;

[[nodiscard]] auto DecodeTreeRequest(std::string_view request_json)
    -> TreeRequestPayload;
[[nodiscard]] auto EncodeTreeRequest(const TreeRequestPayload& request)
    -> std::string;
[[nodiscard]] auto DecodeTreeResponse(std::string_view response_json)
    -> TreeResponsePayload;
[[nodiscard]] auto EncodeTreeResponse(const TreeResponsePayload& response)
    -> std::string;

}  // namespace tracer::transport

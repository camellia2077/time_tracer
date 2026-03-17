import tracer.transport.fields;

#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;
using tracer::transport::modfields::RequireStringField;
using tracer::transport::modfields::TryReadIntListField;
using tracer::transport::modfields::TryReadStringField;

auto ParseRequestObject(std::string_view request_json) -> json {
  if (request_json.empty()) {
    throw std::invalid_argument("request_json must not be empty.");
  }
  json payload = json::parse(std::string(request_json));
  if (!payload.is_object()) {
    throw std::invalid_argument("request_json must be a JSON object.");
  }
  return payload;
}

}  // namespace

auto DecodeReportRequest(std::string_view request_json) -> ReportRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kType = RequireStringField(kPayload, "type");
  const auto kArgument = RequireStringField(kPayload, "argument");
  const auto kFormat = TryReadStringField(kPayload, "format");
  if (kType.HasError()) {
    throw std::invalid_argument(kType.error.message);
  }
  if (kArgument.HasError()) {
    throw std::invalid_argument(kArgument.error.message);
  }
  if (kFormat.HasError()) {
    throw std::invalid_argument(kFormat.error.message);
  }

  ReportRequestPayload out{};
  out.type = kType.value.value_or("");
  out.argument = kArgument.value.value_or("");
  out.format = kFormat.value;
  return out;
}

auto EncodeReportRequest(const ReportRequestPayload& request) -> std::string {
  json payload = {
      {"type", request.type},
      {"argument", request.argument},
  };
  if (request.format.has_value()) {
    payload["format"] = *request.format;
  }
  return payload.dump();
}

auto EncodeReportResponse(const ReportResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"content", response.content},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

auto DecodeReportBatchRequest(std::string_view request_json)
    -> ReportBatchRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kDaysList = TryReadIntListField(kPayload, "days_list");
  if (kDaysList.HasError()) {
    throw std::invalid_argument(kDaysList.error.message);
  }
  if (!kDaysList.value.has_value()) {
    throw std::invalid_argument("field `days_list` must be an integer array.");
  }

  const auto kFormat = TryReadStringField(kPayload, "format");
  if (kFormat.HasError()) {
    throw std::invalid_argument(kFormat.error.message);
  }

  ReportBatchRequestPayload out{};
  out.days_list = *kDaysList.value;
  out.format = kFormat.value;
  return out;
}

auto EncodeReportBatchRequest(const ReportBatchRequestPayload& request)
    -> std::string {
  json payload = {
      {"days_list", request.days_list},
  };
  if (request.format.has_value()) {
    payload["format"] = *request.format;
  }
  return payload.dump();
}

auto EncodeReportBatchResponse(const ReportBatchResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"content", response.content},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

}  // namespace tracer::transport

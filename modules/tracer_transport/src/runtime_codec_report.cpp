#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "tracer/transport/fields.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;

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
  const json payload = ParseRequestObject(request_json);

  const auto type = RequireStringField(payload, "type");
  const auto argument = RequireStringField(payload, "argument");
  const auto format = TryReadStringField(payload, "format");
  if (type.HasError()) {
    throw std::invalid_argument(type.error.message);
  }
  if (argument.HasError()) {
    throw std::invalid_argument(argument.error.message);
  }
  if (format.HasError()) {
    throw std::invalid_argument(format.error.message);
  }

  ReportRequestPayload out{};
  out.type = *type.value;
  out.argument = *argument.value;
  out.format = format.value;
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
  }
      .dump();
}

auto DecodeReportBatchRequest(std::string_view request_json)
    -> ReportBatchRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto days_list = TryReadIntListField(payload, "days_list");
  if (days_list.HasError()) {
    throw std::invalid_argument(days_list.error.message);
  }
  if (!days_list.value.has_value()) {
    throw std::invalid_argument("field `days_list` must be an integer array.");
  }

  const auto format = TryReadStringField(payload, "format");
  if (format.HasError()) {
    throw std::invalid_argument(format.error.message);
  }

  ReportBatchRequestPayload out{};
  out.days_list = *days_list.value;
  out.format = format.value;
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
  }
      .dump();
}

}  // namespace tracer::transport

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

auto DecodeExportRequest(std::string_view request_json) -> ExportRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kType = RequireStringField(kPayload, "type");
  if (kType.HasError()) {
    throw std::invalid_argument(kType.error.message);
  }
  const auto kArgument = TryReadStringField(kPayload, "argument");
  if (kArgument.HasError()) {
    throw std::invalid_argument(kArgument.error.message);
  }
  const auto kFormat = TryReadStringField(kPayload, "format");
  if (kFormat.HasError()) {
    throw std::invalid_argument(kFormat.error.message);
  }
  const auto kRecentDays =
      TryReadIntListField(kPayload, "recent_days_list");
  if (kRecentDays.HasError()) {
    throw std::invalid_argument(kRecentDays.error.message);
  }

  ExportRequestPayload out{};
  out.type = kType.value.value_or("");
  out.argument = kArgument.value;
  out.format = kFormat.value;
  out.recent_days_list = kRecentDays.value;
  return out;
}

auto EncodeExportRequest(const ExportRequestPayload& request) -> std::string {
  json payload = {
      {"type", request.type},
  };
  if (request.argument.has_value()) {
    payload["argument"] = *request.argument;
  }
  if (request.format.has_value()) {
    payload["format"] = *request.format;
  }
  if (request.recent_days_list.has_value()) {
    payload["recent_days_list"] = *request.recent_days_list;
  }
  return payload.dump();
}

auto EncodeExportResponse(const ExportResponsePayload& response)
    -> std::string {
  return nlohmann::json{
      {"ok", response.ok},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

}  // namespace tracer::transport

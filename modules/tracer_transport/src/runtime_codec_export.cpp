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

auto DecodeExportRequest(std::string_view request_json) -> ExportRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto type = RequireStringField(payload, "type");
  if (type.HasError()) {
    throw std::invalid_argument(type.error.message);
  }
  const auto argument = TryReadStringField(payload, "argument");
  if (argument.HasError()) {
    throw std::invalid_argument(argument.error.message);
  }
  const auto format = TryReadStringField(payload, "format");
  if (format.HasError()) {
    throw std::invalid_argument(format.error.message);
  }
  const auto recent_days = TryReadIntListField(payload, "recent_days_list");
  if (recent_days.HasError()) {
    throw std::invalid_argument(recent_days.error.message);
  }

  ExportRequestPayload out{};
  out.type = *type.value;
  out.argument = argument.value;
  out.format = format.value;
  out.recent_days_list = recent_days.value;
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
  }
      .dump();
}

}  // namespace tracer::transport

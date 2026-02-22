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

auto DecodeIngestRequest(std::string_view request_json) -> IngestRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto input_path = RequireStringField(payload, "input_path");
  if (input_path.HasError()) {
    throw std::invalid_argument(input_path.error.message);
  }

  IngestRequestPayload out{};
  out.input_path = *input_path.value;

  const auto date_check_mode = TryReadStringField(payload, "date_check_mode");
  if (date_check_mode.HasError()) {
    throw std::invalid_argument(date_check_mode.error.message);
  }
  out.date_check_mode = date_check_mode.value;

  const auto save_processed =
      TryReadBoolField(payload, "save_processed_output");
  if (save_processed.HasError()) {
    throw std::invalid_argument(save_processed.error.message);
  }
  out.save_processed_output = save_processed.value;

  const auto ingest_mode = TryReadStringField(payload, "ingest_mode");
  if (ingest_mode.HasError()) {
    throw std::invalid_argument(ingest_mode.error.message);
  }
  out.ingest_mode = ingest_mode.value;

  return out;
}

auto EncodeIngestRequest(const IngestRequestPayload& request) -> std::string {
  json payload = {
      {"input_path", request.input_path},
  };
  if (request.date_check_mode.has_value()) {
    payload["date_check_mode"] = *request.date_check_mode;
  }
  if (request.save_processed_output.has_value()) {
    payload["save_processed_output"] = *request.save_processed_output;
  }
  if (request.ingest_mode.has_value()) {
    payload["ingest_mode"] = *request.ingest_mode;
  }
  return payload.dump();
}

auto EncodeIngestResponse(const IngestResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"error_message", response.error_message},
  }
      .dump();
}

}  // namespace tracer::transport

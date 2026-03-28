#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

import tracer.transport.fields;

namespace tracer::transport {

namespace {

using nlohmann::json;
using tracer::transport::modfields::RequireStringField;
using tracer::transport::modfields::TryReadBoolField;
using tracer::transport::modfields::TryReadStringField;
using tracer::transport::modfields::TryReadStringListField;

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

auto DecodeIngestRequest(std::string_view request_json)
    -> IngestRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kInputPath = RequireStringField(kPayload, "input_path");
  if (kInputPath.HasError()) {
    throw std::invalid_argument(kInputPath.error.message);
  }

  IngestRequestPayload out{};
  out.input_path = kInputPath.value.value_or("");

  const auto kDateCheckMode = TryReadStringField(kPayload, "date_check_mode");
  if (kDateCheckMode.HasError()) {
    throw std::invalid_argument(kDateCheckMode.error.message);
  }
  out.date_check_mode = kDateCheckMode.value;

  const auto kSaveProcessed =
      TryReadBoolField(kPayload, "save_processed_output");
  if (kSaveProcessed.HasError()) {
    throw std::invalid_argument(kSaveProcessed.error.message);
  }
  out.save_processed_output = kSaveProcessed.value;

  const auto kIngestMode = TryReadStringField(kPayload, "ingest_mode");
  if (kIngestMode.HasError()) {
    throw std::invalid_argument(kIngestMode.error.message);
  }
  out.ingest_mode = kIngestMode.value;

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
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

auto DecodeIngestSyncStatusRequest(std::string_view request_json)
    -> IngestSyncStatusRequestPayload {
  if (request_json.empty()) {
    return IngestSyncStatusRequestPayload{};
  }

  const json kPayload = ParseRequestObject(request_json);
  const auto kMonths = TryReadStringListField(kPayload, "months");
  if (kMonths.HasError()) {
    throw std::invalid_argument(kMonths.error.message);
  }

  IngestSyncStatusRequestPayload out{};
  out.months = kMonths.value;
  return out;
}

auto EncodeIngestSyncStatusRequest(
    const IngestSyncStatusRequestPayload& request) -> std::string {
  json payload = json::object();
  if (request.months.has_value()) {
    payload["months"] = *request.months;
  }
  return payload.dump();
}

auto EncodeIngestSyncStatusResponse(
    const IngestSyncStatusResponsePayload& response) -> std::string {
  json items = json::array();
  for (const auto& item : response.items) {
    items.push_back(json{
        {"month_key", item.month_key},
        {"txt_relative_path", item.txt_relative_path},
        {"txt_content_hash_sha256", item.txt_content_hash_sha256},
        {"ingested_at_unix_ms", item.ingested_at_unix_ms},
    });
  }

  return json{
      {"ok", response.ok},
      {"items", std::move(items)},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

}  // namespace tracer::transport

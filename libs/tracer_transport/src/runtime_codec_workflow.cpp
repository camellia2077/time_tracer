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

auto DecodeConvertRequest(std::string_view request_json)
    -> ConvertRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kInputPath = RequireStringField(kPayload, "input_path");
  if (kInputPath.HasError()) {
    throw std::invalid_argument(kInputPath.error.message);
  }

  const auto kDateCheckMode = TryReadStringField(kPayload, "date_check_mode");
  const auto kSaveProcessed =
      TryReadBoolField(kPayload, "save_processed_output");
  const auto kValidateLogic = TryReadBoolField(kPayload, "validate_logic");
  const auto kValidateStructure =
      TryReadBoolField(kPayload, "validate_structure");
  if (kDateCheckMode.HasError()) {
    throw std::invalid_argument(kDateCheckMode.error.message);
  }
  if (kSaveProcessed.HasError()) {
    throw std::invalid_argument(kSaveProcessed.error.message);
  }
  if (kValidateLogic.HasError()) {
    throw std::invalid_argument(kValidateLogic.error.message);
  }
  if (kValidateStructure.HasError()) {
    throw std::invalid_argument(kValidateStructure.error.message);
  }

  ConvertRequestPayload out{};
  out.input_path = kInputPath.value.value_or("");
  out.date_check_mode = kDateCheckMode.value;
  out.save_processed_output = kSaveProcessed.value;
  out.validate_logic = kValidateLogic.value;
  out.validate_structure = kValidateStructure.value;
  return out;
}

auto EncodeConvertRequest(const ConvertRequestPayload& request) -> std::string {
  json payload = {
      {"input_path", request.input_path},
  };
  if (request.date_check_mode.has_value()) {
    payload["date_check_mode"] = *request.date_check_mode;
  }
  if (request.save_processed_output.has_value()) {
    payload["save_processed_output"] = *request.save_processed_output;
  }
  if (request.validate_logic.has_value()) {
    payload["validate_logic"] = *request.validate_logic;
  }
  if (request.validate_structure.has_value()) {
    payload["validate_structure"] = *request.validate_structure;
  }
  return payload.dump();
}

auto DecodeImportRequest(std::string_view request_json)
    -> ImportRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kProcessedPath = RequireStringField(kPayload, "processed_path");
  if (kProcessedPath.HasError()) {
    throw std::invalid_argument(kProcessedPath.error.message);
  }

  ImportRequestPayload out{};
  out.processed_path = kProcessedPath.value.value_or("");
  return out;
}

auto EncodeImportRequest(const ImportRequestPayload& request) -> std::string {
  return json{
      {"processed_path", request.processed_path},
  }
      .dump();
}

auto DecodeValidateStructureRequest(std::string_view request_json)
    -> ValidateStructureRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kInputPath = RequireStringField(kPayload, "input_path");
  if (kInputPath.HasError()) {
    throw std::invalid_argument(kInputPath.error.message);
  }

  ValidateStructureRequestPayload out{};
  out.input_path = kInputPath.value.value_or("");
  return out;
}

auto EncodeValidateStructureRequest(
    const ValidateStructureRequestPayload& request) -> std::string {
  return json{
      {"input_path", request.input_path},
  }
      .dump();
}

auto DecodeValidateLogicRequest(std::string_view request_json)
    -> ValidateLogicRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kInputPath = RequireStringField(kPayload, "input_path");
  if (kInputPath.HasError()) {
    throw std::invalid_argument(kInputPath.error.message);
  }
  const auto kDateCheckMode = TryReadStringField(kPayload, "date_check_mode");
  if (kDateCheckMode.HasError()) {
    throw std::invalid_argument(kDateCheckMode.error.message);
  }

  ValidateLogicRequestPayload out{};
  out.input_path = kInputPath.value.value_or("");
  out.date_check_mode = kDateCheckMode.value;
  return out;
}

auto EncodeValidateLogicRequest(const ValidateLogicRequestPayload& request)
    -> std::string {
  json payload = {
      {"input_path", request.input_path},
  };
  if (request.date_check_mode.has_value()) {
    payload["date_check_mode"] = *request.date_check_mode;
  }
  return payload.dump();
}

}  // namespace tracer::transport

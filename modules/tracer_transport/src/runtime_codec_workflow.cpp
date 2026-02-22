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

auto DecodeConvertRequest(std::string_view request_json)
    -> ConvertRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto input_path = RequireStringField(payload, "input_path");
  if (input_path.HasError()) {
    throw std::invalid_argument(input_path.error.message);
  }

  const auto date_check_mode = TryReadStringField(payload, "date_check_mode");
  const auto save_processed =
      TryReadBoolField(payload, "save_processed_output");
  const auto validate_logic = TryReadBoolField(payload, "validate_logic");
  const auto validate_structure =
      TryReadBoolField(payload, "validate_structure");
  if (date_check_mode.HasError()) {
    throw std::invalid_argument(date_check_mode.error.message);
  }
  if (save_processed.HasError()) {
    throw std::invalid_argument(save_processed.error.message);
  }
  if (validate_logic.HasError()) {
    throw std::invalid_argument(validate_logic.error.message);
  }
  if (validate_structure.HasError()) {
    throw std::invalid_argument(validate_structure.error.message);
  }

  ConvertRequestPayload out{};
  out.input_path = *input_path.value;
  out.date_check_mode = date_check_mode.value;
  out.save_processed_output = save_processed.value;
  out.validate_logic = validate_logic.value;
  out.validate_structure = validate_structure.value;
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

auto DecodeImportRequest(std::string_view request_json) -> ImportRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto processed_path = RequireStringField(payload, "processed_path");
  if (processed_path.HasError()) {
    throw std::invalid_argument(processed_path.error.message);
  }

  ImportRequestPayload out{};
  out.processed_path = *processed_path.value;
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
  const json payload = ParseRequestObject(request_json);

  const auto input_path = RequireStringField(payload, "input_path");
  if (input_path.HasError()) {
    throw std::invalid_argument(input_path.error.message);
  }

  ValidateStructureRequestPayload out{};
  out.input_path = *input_path.value;
  return out;
}

auto EncodeValidateStructureRequest(const ValidateStructureRequestPayload& request)
    -> std::string {
  return json{
      {"input_path", request.input_path},
  }
      .dump();
}

auto DecodeValidateLogicRequest(std::string_view request_json)
    -> ValidateLogicRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto input_path = RequireStringField(payload, "input_path");
  if (input_path.HasError()) {
    throw std::invalid_argument(input_path.error.message);
  }
  const auto date_check_mode = TryReadStringField(payload, "date_check_mode");
  if (date_check_mode.HasError()) {
    throw std::invalid_argument(date_check_mode.error.message);
  }

  ValidateLogicRequestPayload out{};
  out.input_path = *input_path.value;
  out.date_check_mode = date_check_mode.value;
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

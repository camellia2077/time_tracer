#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "tracer/transport/runtime_codec.hpp"

import tracer.transport;

namespace tracer::transport {

namespace {

using nlohmann::json;
using tracer::transport::modfields::RequireStringField;
using tracer::transport::modfields::TryReadBoolField;
using tracer::transport::modfields::TryReadStringField;

auto ParseEnvelope(std::string_view response_json, std::string_view context) {
  return tracer::transport::modenvelope::Parse({
      .response_json = response_json,
      .context = context,
  });
}

auto ParseResponseObject(std::string_view response_json) -> json {
  if (response_json.empty()) {
    throw std::invalid_argument("response_json must not be empty.");
  }
  json payload = json::parse(std::string(response_json));
  if (!payload.is_object()) {
    throw std::invalid_argument("response_json must be a JSON object.");
  }
  return payload;
}

auto ParseCliGlobalDefaults(const json& payload) -> CliGlobalDefaultsPayload {
  CliGlobalDefaultsPayload out{};
  const auto kDefaultFormat = TryReadStringField(payload, "default_format");
  if (kDefaultFormat.HasError()) {
    throw std::invalid_argument(kDefaultFormat.error.message);
  }
  out.default_format = kDefaultFormat.value;
  return out;
}

auto ParseCliCommandDefaults(const json& payload) -> CliCommandDefaultsPayload {
  CliCommandDefaultsPayload out{};

  const auto kExportFormat = TryReadStringField(payload, "export_format");
  const auto kQueryFormat = TryReadStringField(payload, "query_format");
  const auto kConvertDateCheckMode =
      TryReadStringField(payload, "convert_date_check_mode");
  const auto kConvertSaveProcessedOutput =
      TryReadBoolField(payload, "convert_save_processed_output");
  const auto kConvertValidateLogic =
      TryReadBoolField(payload, "convert_validate_logic");
  const auto kConvertValidateStructure =
      TryReadBoolField(payload, "convert_validate_structure");
  const auto kIngestDateCheckMode =
      TryReadStringField(payload, "ingest_date_check_mode");
  const auto kIngestSaveProcessedOutput =
      TryReadBoolField(payload, "ingest_save_processed_output");
  const auto kValidateLogicDateCheckMode =
      TryReadStringField(payload, "validate_logic_date_check_mode");

  if (kExportFormat.HasError()) {
    throw std::invalid_argument(kExportFormat.error.message);
  }
  if (kQueryFormat.HasError()) {
    throw std::invalid_argument(kQueryFormat.error.message);
  }
  if (kConvertDateCheckMode.HasError()) {
    throw std::invalid_argument(kConvertDateCheckMode.error.message);
  }
  if (kConvertSaveProcessedOutput.HasError()) {
    throw std::invalid_argument(kConvertSaveProcessedOutput.error.message);
  }
  if (kConvertValidateLogic.HasError()) {
    throw std::invalid_argument(kConvertValidateLogic.error.message);
  }
  if (kConvertValidateStructure.HasError()) {
    throw std::invalid_argument(kConvertValidateStructure.error.message);
  }
  if (kIngestDateCheckMode.HasError()) {
    throw std::invalid_argument(kIngestDateCheckMode.error.message);
  }
  if (kIngestSaveProcessedOutput.HasError()) {
    throw std::invalid_argument(kIngestSaveProcessedOutput.error.message);
  }
  if (kValidateLogicDateCheckMode.HasError()) {
    throw std::invalid_argument(kValidateLogicDateCheckMode.error.message);
  }

  out.export_format = kExportFormat.value;
  out.query_format = kQueryFormat.value;
  out.convert_date_check_mode = kConvertDateCheckMode.value;
  out.convert_save_processed_output = kConvertSaveProcessedOutput.value;
  out.convert_validate_logic = kConvertValidateLogic.value;
  out.convert_validate_structure = kConvertValidateStructure.value;
  out.ingest_date_check_mode = kIngestDateCheckMode.value;
  out.ingest_save_processed_output = kIngestSaveProcessedOutput.value;
  out.validate_logic_date_check_mode = kValidateLogicDateCheckMode.value;
  return out;
}

auto ParseCliConfig(const json& payload) -> CliConfigPayload {
  CliConfigPayload out{};

  const auto kDefaultSave =
      TryReadBoolField(payload, "default_save_processed_output");
  const auto kDefaultDateMode =
      TryReadStringField(payload, "default_date_check_mode");
  if (kDefaultSave.HasError()) {
    throw std::invalid_argument(kDefaultSave.error.message);
  }
  if (kDefaultDateMode.HasError()) {
    throw std::invalid_argument(kDefaultDateMode.error.message);
  }
  out.default_save_processed_output = kDefaultSave.value.value_or(false);
  out.default_date_check_mode = kDefaultDateMode.value;

  if (const auto kDefaultsIt = payload.find("defaults");
      kDefaultsIt != payload.end() && !kDefaultsIt->is_null()) {
    if (!kDefaultsIt->is_object()) {
      throw std::invalid_argument("field `defaults` must be an object.");
    }
    out.defaults = ParseCliGlobalDefaults(*kDefaultsIt);
  }

  if (const auto kCommandDefaultsIt = payload.find("command_defaults");
      kCommandDefaultsIt != payload.end() && !kCommandDefaultsIt->is_null()) {
    if (!kCommandDefaultsIt->is_object()) {
      throw std::invalid_argument(
          "field `command_defaults` must be an object.");
    }
    out.command_defaults = ParseCliCommandDefaults(*kCommandDefaultsIt);
  }

  return out;
}

auto ParseResolvedCliPaths(const json& payload) -> ResolvedCliPathsPayload {
  const auto kDbPath = RequireStringField(payload, "db_path");
  const auto kRuntimeOutputRoot =
      RequireStringField(payload, "runtime_output_root");
  const auto kConverterConfigTomlPath =
      RequireStringField(payload, "converter_config_toml_path");

  if (kDbPath.HasError()) {
    throw std::invalid_argument(kDbPath.error.message);
  }
  if (kRuntimeOutputRoot.HasError()) {
    throw std::invalid_argument(kRuntimeOutputRoot.error.message);
  }
  if (kConverterConfigTomlPath.HasError()) {
    throw std::invalid_argument(kConverterConfigTomlPath.error.message);
  }

  const auto kExeDir = TryReadStringField(payload, "exe_dir");
  const auto kOutputRoot = TryReadStringField(payload, "output_root");
  const auto kExportRoot = TryReadStringField(payload, "export_root");

  if (kExeDir.HasError()) {
    throw std::invalid_argument(kExeDir.error.message);
  }
  if (kOutputRoot.HasError()) {
    throw std::invalid_argument(kOutputRoot.error.message);
  }
  if (kExportRoot.HasError()) {
    throw std::invalid_argument(kExportRoot.error.message);
  }

  ResolvedCliPathsPayload out{};
  out.db_path = *kDbPath.value;
  out.runtime_output_root = *kRuntimeOutputRoot.value;
  out.converter_config_toml_path = *kConverterConfigTomlPath.value;
  out.exe_dir = kExeDir.value;
  out.output_root = kOutputRoot.value;
  out.export_root = kExportRoot.value;
  return out;
}

}  // namespace

auto DecodeAckResponse(std::string_view response_json, std::string_view context)
    -> AckResponsePayload {
  const auto kParsed = ParseEnvelope(response_json, context);
  if (kParsed.HasError()) {
    throw std::invalid_argument(kParsed.error.message);
  }

  AckResponsePayload out{};
  out.ok = kParsed.envelope.ok;
  out.error_message = kParsed.envelope.error_message;
  out.error_contract.error_code = kParsed.envelope.error_code;
  out.error_contract.error_category = kParsed.envelope.error_category;
  out.error_contract.hints = kParsed.envelope.hints;
  if (!out.ok && out.error_message.empty()) {
    out.error_message = "Core operation failed.";
  }
  return out;
}

auto DecodeTextResponse(std::string_view response_json,
                        std::string_view context) -> TextResponsePayload {
  const auto kParsed = ParseEnvelope(response_json, context);
  if (kParsed.HasError()) {
    throw std::invalid_argument(kParsed.error.message);
  }

  TextResponsePayload out{};
  out.ok = kParsed.envelope.ok;
  out.error_message = kParsed.envelope.error_message;
  out.content = kParsed.envelope.content;
  out.error_contract.error_code = kParsed.envelope.error_code;
  out.error_contract.error_category = kParsed.envelope.error_category;
  out.error_contract.hints = kParsed.envelope.hints;
  if (!out.ok && out.error_message.empty()) {
    out.error_message = "Core operation failed.";
  }
  return out;
}

auto DecodeRuntimeCheckResponse(std::string_view response_json)
    -> RuntimeCheckResponsePayload {
  const json kPayload = ParseResponseObject(response_json);

  const auto kOk = TryReadBoolField(kPayload, "ok");
  const auto kErrorMessage = TryReadStringField(kPayload, "error_message");
  const auto kErrorCode = TryReadStringField(kPayload, "error_code");
  const auto kErrorCategory = TryReadStringField(kPayload, "error_category");

  if (kOk.HasError()) {
    throw std::invalid_argument(kOk.error.message);
  }
  if (!kOk.value.has_value()) {
    throw std::invalid_argument("field `ok` must be a boolean.");
  }
  if (kErrorMessage.HasError()) {
    throw std::invalid_argument(kErrorMessage.error.message);
  }
  if (kErrorCode.HasError()) {
    throw std::invalid_argument(kErrorCode.error.message);
  }
  if (kErrorCategory.HasError()) {
    throw std::invalid_argument(kErrorCategory.error.message);
  }

  RuntimeCheckResponsePayload out{};
  out.ok = *kOk.value;
  out.error_message = kErrorMessage.value.value_or("");
  out.error_contract.error_code = kErrorCode.value.value_or("");
  out.error_contract.error_category = kErrorCategory.value.value_or("");

  if (const auto kMessagesIt = kPayload.find("messages");
      kMessagesIt != kPayload.end() && !kMessagesIt->is_null()) {
    if (!kMessagesIt->is_array()) {
      throw std::invalid_argument("field `messages` must be a string array.");
    }
    out.messages.reserve(kMessagesIt->size());
    for (const auto& item : *kMessagesIt) {
      if (!item.is_string()) {
        throw std::invalid_argument("field `messages` must be a string array.");
      }
      out.messages.push_back(item.get<std::string>());
    }
  }

  if (const auto kHintsIt = kPayload.find("hints");
      kHintsIt != kPayload.end() && !kHintsIt->is_null()) {
    if (!kHintsIt->is_array()) {
      throw std::invalid_argument("field `hints` must be a string array.");
    }
    out.error_contract.hints.reserve(kHintsIt->size());
    for (const auto& item : *kHintsIt) {
      if (!item.is_string()) {
        throw std::invalid_argument("field `hints` must be a string array.");
      }
      out.error_contract.hints.push_back(item.get<std::string>());
    }
  }

  return out;
}

auto DecodeResolveCliContextResponse(std::string_view response_json)
    -> ResolveCliContextResponsePayload {
  const json kPayload = ParseResponseObject(response_json);

  const auto kOk = TryReadBoolField(kPayload, "ok");
  const auto kErrorMessage = TryReadStringField(kPayload, "error_message");
  const auto kErrorCode = TryReadStringField(kPayload, "error_code");
  const auto kErrorCategory = TryReadStringField(kPayload, "error_category");

  if (kOk.HasError()) {
    throw std::invalid_argument(kOk.error.message);
  }
  if (!kOk.value.has_value()) {
    throw std::invalid_argument("field `ok` must be a boolean.");
  }
  if (kErrorMessage.HasError()) {
    throw std::invalid_argument(kErrorMessage.error.message);
  }
  if (kErrorCode.HasError()) {
    throw std::invalid_argument(kErrorCode.error.message);
  }
  if (kErrorCategory.HasError()) {
    throw std::invalid_argument(kErrorCategory.error.message);
  }

  ResolveCliContextResponsePayload out{};
  out.ok = *kOk.value;
  out.error_message = kErrorMessage.value.value_or("");
  out.error_contract.error_code = kErrorCode.value.value_or("");
  out.error_contract.error_category = kErrorCategory.value.value_or("");
  if (!out.ok) {
    if (const auto kHintsIt = kPayload.find("hints");
        kHintsIt != kPayload.end() && kHintsIt->is_array()) {
      out.error_contract.hints.reserve(kHintsIt->size());
      for (const auto& item : *kHintsIt) {
        if (item.is_string()) {
          out.error_contract.hints.push_back(item.get<std::string>());
        }
      }
    }
    return out;
  }

  const auto kPathsIt = kPayload.find("paths");
  if (kPathsIt == kPayload.end() || !kPathsIt->is_object()) {
    throw std::invalid_argument(
        "field `paths` must be an object when `ok=true`.");
  }
  out.paths = ParseResolvedCliPaths(*kPathsIt);

  const auto kCliConfigIt = kPayload.find("cli_config");
  if (kCliConfigIt == kPayload.end() || !kCliConfigIt->is_object()) {
    throw std::invalid_argument(
        "field `cli_config` must be an object when `ok=true`.");
  }
  out.cli_config = ParseCliConfig(*kCliConfigIt);

  return out;
}

}  // namespace tracer::transport

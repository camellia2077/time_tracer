import tracer.transport.envelope;
import tracer.transport.fields;

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "tracer/transport/runtime_codec.hpp"

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
  const auto default_format = TryReadStringField(payload, "default_format");
  if (default_format.HasError()) {
    throw std::invalid_argument(default_format.error.message);
  }
  out.default_format = default_format.value;
  return out;
}

auto ParseCliCommandDefaults(const json& payload) -> CliCommandDefaultsPayload {
  CliCommandDefaultsPayload out{};

  const auto export_format = TryReadStringField(payload, "export_format");
  const auto query_format = TryReadStringField(payload, "query_format");
  const auto convert_date_check_mode =
      TryReadStringField(payload, "convert_date_check_mode");
  const auto convert_save_processed_output =
      TryReadBoolField(payload, "convert_save_processed_output");
  const auto convert_validate_logic =
      TryReadBoolField(payload, "convert_validate_logic");
  const auto convert_validate_structure =
      TryReadBoolField(payload, "convert_validate_structure");
  const auto ingest_date_check_mode =
      TryReadStringField(payload, "ingest_date_check_mode");
  const auto ingest_save_processed_output =
      TryReadBoolField(payload, "ingest_save_processed_output");
  const auto validate_logic_date_check_mode =
      TryReadStringField(payload, "validate_logic_date_check_mode");

  if (export_format.HasError()) {
    throw std::invalid_argument(export_format.error.message);
  }
  if (query_format.HasError()) {
    throw std::invalid_argument(query_format.error.message);
  }
  if (convert_date_check_mode.HasError()) {
    throw std::invalid_argument(convert_date_check_mode.error.message);
  }
  if (convert_save_processed_output.HasError()) {
    throw std::invalid_argument(convert_save_processed_output.error.message);
  }
  if (convert_validate_logic.HasError()) {
    throw std::invalid_argument(convert_validate_logic.error.message);
  }
  if (convert_validate_structure.HasError()) {
    throw std::invalid_argument(convert_validate_structure.error.message);
  }
  if (ingest_date_check_mode.HasError()) {
    throw std::invalid_argument(ingest_date_check_mode.error.message);
  }
  if (ingest_save_processed_output.HasError()) {
    throw std::invalid_argument(ingest_save_processed_output.error.message);
  }
  if (validate_logic_date_check_mode.HasError()) {
    throw std::invalid_argument(validate_logic_date_check_mode.error.message);
  }

  out.export_format = export_format.value;
  out.query_format = query_format.value;
  out.convert_date_check_mode = convert_date_check_mode.value;
  out.convert_save_processed_output = convert_save_processed_output.value;
  out.convert_validate_logic = convert_validate_logic.value;
  out.convert_validate_structure = convert_validate_structure.value;
  out.ingest_date_check_mode = ingest_date_check_mode.value;
  out.ingest_save_processed_output = ingest_save_processed_output.value;
  out.validate_logic_date_check_mode = validate_logic_date_check_mode.value;
  return out;
}

auto ParseCliConfig(const json& payload) -> CliConfigPayload {
  CliConfigPayload out{};

  const auto default_save =
      TryReadBoolField(payload, "default_save_processed_output");
  const auto default_date_mode =
      TryReadStringField(payload, "default_date_check_mode");
  if (default_save.HasError()) {
    throw std::invalid_argument(default_save.error.message);
  }
  if (default_date_mode.HasError()) {
    throw std::invalid_argument(default_date_mode.error.message);
  }
  out.default_save_processed_output = default_save.value.value_or(false);
  out.default_date_check_mode = default_date_mode.value;

  if (const auto defaults_it = payload.find("defaults");
      defaults_it != payload.end() && !defaults_it->is_null()) {
    if (!defaults_it->is_object()) {
      throw std::invalid_argument("field `defaults` must be an object.");
    }
    out.defaults = ParseCliGlobalDefaults(*defaults_it);
  }

  if (const auto command_defaults_it = payload.find("command_defaults");
      command_defaults_it != payload.end() && !command_defaults_it->is_null()) {
    if (!command_defaults_it->is_object()) {
      throw std::invalid_argument(
          "field `command_defaults` must be an object.");
    }
    out.command_defaults = ParseCliCommandDefaults(*command_defaults_it);
  }

  return out;
}

auto ParseResolvedCliPaths(const json& payload) -> ResolvedCliPathsPayload {
  const auto db_path = RequireStringField(payload, "db_path");
  const auto runtime_output_root =
      RequireStringField(payload, "runtime_output_root");
  const auto converter_config_toml_path =
      RequireStringField(payload, "converter_config_toml_path");

  if (db_path.HasError()) {
    throw std::invalid_argument(db_path.error.message);
  }
  if (runtime_output_root.HasError()) {
    throw std::invalid_argument(runtime_output_root.error.message);
  }
  if (converter_config_toml_path.HasError()) {
    throw std::invalid_argument(converter_config_toml_path.error.message);
  }

  const auto exe_dir = TryReadStringField(payload, "exe_dir");
  const auto output_root = TryReadStringField(payload, "output_root");
  const auto export_root = TryReadStringField(payload, "export_root");

  if (exe_dir.HasError()) {
    throw std::invalid_argument(exe_dir.error.message);
  }
  if (output_root.HasError()) {
    throw std::invalid_argument(output_root.error.message);
  }
  if (export_root.HasError()) {
    throw std::invalid_argument(export_root.error.message);
  }

  ResolvedCliPathsPayload out{};
  out.db_path = *db_path.value;
  out.runtime_output_root = *runtime_output_root.value;
  out.converter_config_toml_path = *converter_config_toml_path.value;
  out.exe_dir = exe_dir.value;
  out.output_root = output_root.value;
  out.export_root = export_root.value;
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

auto DecodeTextResponse(std::string_view response_json, std::string_view context)
    -> TextResponsePayload {
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
  const json payload = ParseResponseObject(response_json);

  const auto kOk = TryReadBoolField(payload, "ok");
  const auto kErrorMessage = TryReadStringField(payload, "error_message");
  const auto kErrorCode = TryReadStringField(payload, "error_code");
  const auto kErrorCategory = TryReadStringField(payload, "error_category");

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
    if (const auto kHintsIt = payload.find("hints");
        kHintsIt != payload.end() && kHintsIt->is_array()) {
      out.error_contract.hints.reserve(kHintsIt->size());
      for (const auto& item : *kHintsIt) {
        if (item.is_string()) {
          out.error_contract.hints.push_back(item.get<std::string>());
        }
      }
    }
    return out;
  }

  const auto kPathsIt = payload.find("paths");
  if (kPathsIt == payload.end() || !kPathsIt->is_object()) {
    throw std::invalid_argument(
        "field `paths` must be an object when `ok=true`.");
  }
  out.paths = ParseResolvedCliPaths(*kPathsIt);

  const auto kCliConfigIt = payload.find("cli_config");
  if (kCliConfigIt == payload.end() || !kCliConfigIt->is_object()) {
    throw std::invalid_argument(
        "field `cli_config` must be an object when `ok=true`.");
  }
  out.cli_config = ParseCliConfig(*kCliConfigIt);

  return out;
}

}  // namespace tracer::transport

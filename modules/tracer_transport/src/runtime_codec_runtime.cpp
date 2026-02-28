#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "tracer/transport/envelope.hpp"
#include "tracer/transport/fields.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;

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
  const auto parsed = ParseResponseEnvelope(response_json, context);
  if (parsed.HasError()) {
    throw std::invalid_argument(parsed.error.message);
  }

  AckResponsePayload out{};
  out.ok = parsed.envelope.ok;
  out.error_message = parsed.envelope.error_message;
  if (!out.ok && out.error_message.empty()) {
    out.error_message = "Core operation failed.";
  }
  return out;
}

auto DecodeTextResponse(std::string_view response_json, std::string_view context)
    -> TextResponsePayload {
  const auto parsed = ParseResponseEnvelope(response_json, context);
  if (parsed.HasError()) {
    throw std::invalid_argument(parsed.error.message);
  }

  TextResponsePayload out{};
  out.ok = parsed.envelope.ok;
  out.error_message = parsed.envelope.error_message;
  out.content = parsed.envelope.content;
  if (!out.ok && out.error_message.empty()) {
    out.error_message = "Core operation failed.";
  }
  return out;
}

auto DecodeRuntimeCheckResponse(std::string_view response_json)
    -> RuntimeCheckResponsePayload {
  const json payload = ParseResponseObject(response_json);

  const auto ok = TryReadBoolField(payload, "ok");
  const auto error_message = TryReadStringField(payload, "error_message");

  if (ok.HasError()) {
    throw std::invalid_argument(ok.error.message);
  }
  if (!ok.value.has_value()) {
    throw std::invalid_argument("field `ok` must be a boolean.");
  }
  if (error_message.HasError()) {
    throw std::invalid_argument(error_message.error.message);
  }

  RuntimeCheckResponsePayload out{};
  out.ok = *ok.value;
  out.error_message = error_message.value.value_or("");

  if (const auto messages_it = payload.find("messages");
      messages_it != payload.end() && !messages_it->is_null()) {
    if (!messages_it->is_array()) {
      throw std::invalid_argument("field `messages` must be a string array.");
    }
    out.messages.reserve(messages_it->size());
    for (const auto& item : *messages_it) {
      if (!item.is_string()) {
        throw std::invalid_argument("field `messages` must be a string array.");
      }
      out.messages.push_back(item.get<std::string>());
    }
  }

  return out;
}

auto DecodeResolveCliContextResponse(std::string_view response_json)
    -> ResolveCliContextResponsePayload {
  const json payload = ParseResponseObject(response_json);

  const auto ok = TryReadBoolField(payload, "ok");
  const auto error_message = TryReadStringField(payload, "error_message");

  if (ok.HasError()) {
    throw std::invalid_argument(ok.error.message);
  }
  if (!ok.value.has_value()) {
    throw std::invalid_argument("field `ok` must be a boolean.");
  }
  if (error_message.HasError()) {
    throw std::invalid_argument(error_message.error.message);
  }

  ResolveCliContextResponsePayload out{};
  out.ok = *ok.value;
  out.error_message = error_message.value.value_or("");
  if (!out.ok) {
    return out;
  }

  const auto paths_it = payload.find("paths");
  if (paths_it == payload.end() || !paths_it->is_object()) {
    throw std::invalid_argument(
        "field `paths` must be an object when `ok=true`.");
  }
  out.paths = ParseResolvedCliPaths(*paths_it);

  const auto cli_config_it = payload.find("cli_config");
  if (cli_config_it == payload.end() || !cli_config_it->is_object()) {
    throw std::invalid_argument(
        "field `cli_config` must be an object when `ok=true`.");
  }
  out.cli_config = ParseCliConfig(*cli_config_it);

  return out;
}

}  // namespace tracer::transport

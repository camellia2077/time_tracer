#include "tracer/transport/envelope.hpp"

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;

auto PrefixContext(std::string_view context, std::string_view message)
    -> std::string {
  if (context.empty()) {
    return std::string(message);
  }
  std::string full_message;
  full_message.reserve(context.size() + message.size() + 2U);
  full_message.append(context);
  full_message.append(": ");
  full_message.append(message);
  return full_message;
}

auto ParseError(std::string_view context, std::string_view message)
    -> ParseResponseEnvelopeResult {
  return ParseResponseEnvelopeResult{
      .envelope = ResponseEnvelope{},
      .error =
          MakeTransportError(TransportErrorCode::kParseFailure,
                             PrefixContext(context, message)),
  };
}

auto ValidationError(std::string_view context, std::string_view message)
    -> ParseResponseEnvelopeResult {
  return ParseResponseEnvelopeResult{
      .envelope = ResponseEnvelope{},
      .error =
          MakeTransportError(TransportErrorCode::kValidationFailure,
                             PrefixContext(context, message)),
  };
}

auto ReadOptionalString(const json& payload, const char* field_name)
    -> std::string {
  const auto it = payload.find(field_name);
  if (it == payload.end() || !it->is_string()) {
    return {};
  }
  return it->get<std::string>();
}

auto ReadOptionalStringValue(const json& payload, const char* field_name)
    -> std::optional<std::string> {
  const auto it = payload.find(field_name);
  if (it == payload.end() || !it->is_string()) {
    return std::nullopt;
  }
  return it->get<std::string>();
}

auto ReadOptionalStringArray(const json& payload, const char* field_name)
    -> std::vector<std::string> {
  const auto it = payload.find(field_name);
  if (it == payload.end() || !it->is_array()) {
    return {};
  }

  std::vector<std::string> out;
  out.reserve(it->size());
  for (const auto& entry : *it) {
    if (!entry.is_string()) {
      return {};
    }
    out.push_back(entry.get<std::string>());
  }
  return out;
}

}  // namespace

auto BuildResponseEnvelope(bool ok, std::string_view error_message,
                           std::string_view content,
                           std::string_view error_code,
                           std::string_view error_category,
                           const std::vector<std::string>& hints)
    -> ResponseEnvelope {
  return ResponseEnvelope{
      .ok = ok,
      .error_message = std::string(error_message),
      .error_code = std::string(error_code),
      .error_category = std::string(error_category),
      .hints = hints,
      .content = std::string(content),
  };
}

auto SerializeResponseEnvelope(const ResponseEnvelope& envelope) -> std::string {
  json payload = {
      {"ok", envelope.ok},
      {"error_message", envelope.error_message},
      {"error_code", envelope.error_code},
      {"error_category", envelope.error_category},
      {"hints", envelope.hints},
      {"content", envelope.content},
  };
  if (envelope.report_hash_sha256.has_value()) {
    payload["report_hash_sha256"] = *envelope.report_hash_sha256;
  }
  return payload.dump();
}

auto ParseResponseEnvelope(std::string_view response_json,
                           std::string_view context)
    -> ParseResponseEnvelopeResult {
  if (response_json.empty()) {
    return ParseError(context, "empty core response.");
  }

  json payload;
  try {
    payload = json::parse(std::string(response_json));
  } catch (const json::parse_error& error) {
    return ParseError(context, error.what());
  }

  if (!payload.is_object()) {
    return ParseError(context, "non-object JSON core response.");
  }

  const auto ok_it = payload.find("ok");
  if (ok_it == payload.end() || !ok_it->is_boolean()) {
    return ValidationError(context, "core response missing boolean `ok`.");
  }

  return ParseResponseEnvelopeResult{
      .envelope =
          ResponseEnvelope{
              .ok = ok_it->get<bool>(),
              .error_message = ReadOptionalString(payload, "error_message"),
              .error_code = ReadOptionalString(payload, "error_code"),
              .error_category = ReadOptionalString(payload, "error_category"),
              .hints = ReadOptionalStringArray(payload, "hints"),
              .content = ReadOptionalString(payload, "content"),
              .report_hash_sha256 =
                  ReadOptionalStringValue(payload, "report_hash_sha256"),
          },
      .error = TransportError{},
  };
}

}  // namespace tracer::transport

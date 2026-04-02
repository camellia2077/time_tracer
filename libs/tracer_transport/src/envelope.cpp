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
      .error = MakeTransportError(TransportErrorCode::kParseFailure,
                                  PrefixContext(context, message)),
  };
}

auto ValidationError(std::string_view context, std::string_view message)
    -> ParseResponseEnvelopeResult {
  return ParseResponseEnvelopeResult{
      .envelope = ResponseEnvelope{},
      .error = MakeTransportError(TransportErrorCode::kValidationFailure,
                                  PrefixContext(context, message)),
  };
}

auto ReadOptionalString(const json& payload, const char* field_name)
    -> std::string {
  const auto kIt = payload.find(field_name);
  if (kIt == payload.end() || !kIt->is_string()) {
    return {};
  }
  return kIt->get<std::string>();
}

auto ReadOptionalStringValue(const json& payload, const char* field_name)
    -> std::optional<std::string> {
  const auto kIt = payload.find(field_name);
  if (kIt == payload.end() || !kIt->is_string()) {
    return std::nullopt;
  }
  return kIt->get<std::string>();
}

auto ReadOptionalStringArray(const json& payload, const char* field_name)
    -> std::vector<std::string> {
  const auto kIt = payload.find(field_name);
  if (kIt == payload.end() || !kIt->is_array()) {
    return {};
  }

  std::vector<std::string> out;
  out.reserve(kIt->size());
  for (const auto& entry : *kIt) {
    if (!entry.is_string()) {
      return {};
    }
    out.push_back(entry.get<std::string>());
  }
  return out;
}

auto ReadOptionalInt(const json& payload, const char* field_name)
    -> std::optional<int> {
  const auto kIt = payload.find(field_name);
  if (kIt == payload.end() || !kIt->is_number_integer()) {
    return std::nullopt;
  }
  return kIt->get<int>();
}

auto ReadOptionalBool(const json& payload, const char* field_name)
    -> std::optional<bool> {
  const auto kIt = payload.find(field_name);
  if (kIt == payload.end() || !kIt->is_boolean()) {
    return std::nullopt;
  }
  return kIt->get<bool>();
}

auto ReadOptionalReportWindowMetadata(const json& payload)
    -> std::optional<ReportWindowMetadataPayload> {
  const auto kHasRecords = ReadOptionalBool(payload, "has_records");
  const auto kMatchedDayCount = ReadOptionalInt(payload, "matched_day_count");
  const auto kMatchedRecordCount =
      ReadOptionalInt(payload, "matched_record_count");
  const auto kRequestedDays = ReadOptionalInt(payload, "requested_days");
  const auto kStartDate = ReadOptionalStringValue(payload, "start_date");
  const auto kEndDate = ReadOptionalStringValue(payload, "end_date");

  if (!kHasRecords.has_value() && !kMatchedDayCount.has_value() &&
      !kMatchedRecordCount.has_value() && !kRequestedDays.has_value() &&
      !kStartDate.has_value() && !kEndDate.has_value()) {
    return std::nullopt;
  }

  return ReportWindowMetadataPayload{
      .has_records = kHasRecords.value_or(false),
      .matched_day_count = kMatchedDayCount.value_or(0),
      .matched_record_count = kMatchedRecordCount.value_or(0),
      .start_date = kStartDate.value_or(""),
      .end_date = kEndDate.value_or(""),
      .requested_days = kRequestedDays.value_or(0),
  };
}

}  // namespace

auto BuildResponseEnvelope(bool is_ok, std::string_view error_message,
                           std::string_view content,
                           std::string_view error_code,
                           std::string_view error_category,
                           const std::vector<std::string>& hints)
    -> ResponseEnvelope {
  return ResponseEnvelope{
      .ok = is_ok,
      .error_message = std::string(error_message),
      .error_code = std::string(error_code),
      .error_category = std::string(error_category),
      .hints = hints,
      .content = std::string(content),
  };
}

auto SerializeResponseEnvelope(const ResponseEnvelope& envelope)
    -> std::string {
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
  if (envelope.report_window_metadata.has_value()) {
    payload["has_records"] = envelope.report_window_metadata->has_records;
    payload["matched_day_count"] =
        envelope.report_window_metadata->matched_day_count;
    payload["matched_record_count"] =
        envelope.report_window_metadata->matched_record_count;
    payload["start_date"] = envelope.report_window_metadata->start_date;
    payload["end_date"] = envelope.report_window_metadata->end_date;
    payload["requested_days"] = envelope.report_window_metadata->requested_days;
  }
  return payload.dump();
}

auto ParseResponseEnvelope(ResponseEnvelopeParseArgs parse_args)
    -> ParseResponseEnvelopeResult {
  const auto kResponseJson = parse_args.response_json;
  const auto kContext = parse_args.context;
  if (kResponseJson.empty()) {
    return ParseError(kContext, "empty core response.");
  }

  json payload;
  try {
    payload = json::parse(std::string(kResponseJson));
  } catch (const json::parse_error& error) {
    return ParseError(kContext, error.what());
  }

  if (!payload.is_object()) {
    return ParseError(kContext, "non-object JSON core response.");
  }

  const auto kOkIt = payload.find("ok");
  if (kOkIt == payload.end() || !kOkIt->is_boolean()) {
    return ValidationError(kContext, "core response missing boolean `ok`.");
  }

  return ParseResponseEnvelopeResult{
      .envelope =
          ResponseEnvelope{
              .ok = kOkIt->get<bool>(),
              .error_message = ReadOptionalString(payload, "error_message"),
              .error_code = ReadOptionalString(payload, "error_code"),
              .error_category = ReadOptionalString(payload, "error_category"),
              .hints = ReadOptionalStringArray(payload, "hints"),
              .content = ReadOptionalString(payload, "content"),
              .report_hash_sha256 =
                  ReadOptionalStringValue(payload, "report_hash_sha256"),
              .report_window_metadata = ReadOptionalReportWindowMetadata(payload),
          },
      .error = TransportError{},
  };
}

}  // namespace tracer::transport

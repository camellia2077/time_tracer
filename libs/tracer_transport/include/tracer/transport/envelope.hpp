#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "tracer/transport/errors.hpp"

namespace tracer::transport {

struct ReportWindowMetadataPayload {
  bool has_records = false;
  int matched_day_count = 0;
  int matched_record_count = 0;
  std::string start_date;
  std::string end_date;
  int requested_days = 0;
};

struct ResponseEnvelope {
  bool ok = false;
  std::string error_message;
  std::string error_code;
  std::string error_category;
  std::vector<std::string> hints;
  std::string content;
  std::optional<std::string> report_hash_sha256;
  std::optional<ReportWindowMetadataPayload> report_window_metadata;
};

struct ParseResponseEnvelopeResult {
  ResponseEnvelope envelope;
  TransportError error;

  [[nodiscard]] auto HasError() const -> bool { return error.HasError(); }
};

struct ResponseEnvelopeParseArgs {
  std::string_view response_json;
  std::string_view context;
};

[[nodiscard]] auto BuildResponseEnvelope(
    bool is_ok, std::string_view error_message, std::string_view content,
    std::string_view error_code = {}, std::string_view error_category = {},
    const std::vector<std::string>& hints = {}) -> ResponseEnvelope;

[[nodiscard]] auto SerializeResponseEnvelope(const ResponseEnvelope& envelope)
    -> std::string;

[[nodiscard]] auto ParseResponseEnvelope(ResponseEnvelopeParseArgs parse_args)
    -> ParseResponseEnvelopeResult;

}  // namespace tracer::transport

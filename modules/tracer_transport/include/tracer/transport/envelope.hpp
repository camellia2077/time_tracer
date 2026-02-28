#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "tracer/transport/errors.hpp"

namespace tracer::transport {

struct ResponseEnvelope {
  bool ok = false;
  std::string error_message;
  std::string content;
  std::optional<std::string> report_hash_sha256;
};

struct ParseResponseEnvelopeResult {
  ResponseEnvelope envelope;
  TransportError error;

  [[nodiscard]] auto HasError() const -> bool { return error.HasError(); }
};

[[nodiscard]] auto BuildResponseEnvelope(bool ok, std::string_view error_message,
                                         std::string_view content)
    -> ResponseEnvelope;

[[nodiscard]] auto SerializeResponseEnvelope(const ResponseEnvelope& envelope)
    -> std::string;

[[nodiscard]] auto ParseResponseEnvelope(std::string_view response_json,
                                         std::string_view context = {})
    -> ParseResponseEnvelopeResult;

}  // namespace tracer::transport

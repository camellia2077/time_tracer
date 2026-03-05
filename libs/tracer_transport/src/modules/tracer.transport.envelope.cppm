module;

#include "tracer/transport/envelope.hpp"

export module tracer.transport.envelope;

export namespace tracer::transport::modenvelope {

using Envelope = ::tracer::transport::ResponseEnvelope;
using ParseResult = ::tracer::transport::ParseResponseEnvelopeResult;

[[nodiscard]] inline auto Build(bool ok, std::string_view error_message,
                                std::string_view content,
                                std::string_view error_code = {},
                                std::string_view error_category = {},
                                const std::vector<std::string>& hints = {})
    -> Envelope {
  return ::tracer::transport::BuildResponseEnvelope(
      ok, error_message, content, error_code, error_category, hints);
}

[[nodiscard]] inline auto Serialize(const Envelope& envelope) -> std::string {
  return ::tracer::transport::SerializeResponseEnvelope(envelope);
}

[[nodiscard]] inline auto Parse(std::string_view response_json,
                                std::string_view context = {})
    -> ParseResult {
  return ::tracer::transport::ParseResponseEnvelope(response_json, context);
}

}  // namespace tracer::transport::modenvelope


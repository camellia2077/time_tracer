module;

#include "tracer/transport/envelope.hpp"

export module tracer.transport;

export import tracer.transport.errors;
export import tracer.transport.fields;

export namespace tracer::transport::modenvelope {

using Envelope = ::tracer::transport::ResponseEnvelope;
using ParseResult = ::tracer::transport::ParseResponseEnvelopeResult;
using ParseArgs = ::tracer::transport::ResponseEnvelopeParseArgs;

[[nodiscard]] inline auto Build(bool is_ok, std::string_view error_message,
                                std::string_view content,
                                std::string_view error_code = {},
                                std::string_view error_category = {},
                                const std::vector<std::string>& hints = {})
    -> Envelope {
  return ::tracer::transport::BuildResponseEnvelope(
      is_ok, error_message, content, error_code, error_category, hints);
}

[[nodiscard]] inline auto Serialize(const Envelope& envelope) -> std::string {
  return ::tracer::transport::SerializeResponseEnvelope(envelope);
}

[[nodiscard]] inline auto Parse(ParseArgs parse_args) -> ParseResult {
  return ::tracer::transport::ParseResponseEnvelope(parse_args);
}

}  // namespace tracer::transport::modenvelope

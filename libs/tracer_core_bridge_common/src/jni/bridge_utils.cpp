#include "jni/bridge_utils.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include "tracer/transport/runtime_codec.hpp"

namespace tracer_core_bridge_common::jni {

namespace tt_transport = tracer::transport;

[[nodiscard]] auto BuildResponseJson(bool is_ok, std::string_view error_message,
                                     std::string_view content) -> std::string {
  return tt_transport::SerializeResponseEnvelope(
      tt_transport::BuildResponseEnvelope(is_ok, error_message, content));
}

[[nodiscard]] auto ParseCoreResponse(const char* response_json,
                                     std::string_view context)
    -> tt_transport::ResponseEnvelope {
  const auto kParsed = tt_transport::ParseResponseEnvelope(
      tt_transport::ResponseEnvelopeParseArgs{
          .response_json = response_json != nullptr
                               ? std::string_view(response_json)
                               : std::string_view{},
          .context = context,
      });
  if (kParsed.HasError()) {
    throw std::runtime_error(kParsed.error.message);
  }
  return kParsed.envelope;
}

}  // namespace tracer_core_bridge_common::jni

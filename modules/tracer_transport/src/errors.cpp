#include "tracer/transport/errors.hpp"

namespace tracer::transport {

auto MakeTransportError(TransportErrorCode code, std::string_view message)
    -> TransportError {
  return TransportError{
      .code = code,
      .message = std::string(message),
  };
}

}  // namespace tracer::transport

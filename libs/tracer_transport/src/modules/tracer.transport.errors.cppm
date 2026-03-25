module;

#include "tracer/transport/errors.hpp"

export module tracer.transport.errors;

export namespace tracer::transport::moderrors {

using Code = ::tracer::transport::TransportErrorCode;
using Error = ::tracer::transport::TransportError;

[[nodiscard]] inline auto Make(Code code, std::string_view message) -> Error {
  return ::tracer::transport::MakeTransportError(code, message);
}

}  // namespace tracer::transport::moderrors

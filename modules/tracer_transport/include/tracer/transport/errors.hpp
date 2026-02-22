#pragma once

#include <string>
#include <string_view>

namespace tracer::transport {

enum class TransportErrorCode {
  kNone = 0,
  kInvalidArgument = 1,
  kParseFailure = 2,
  kValidationFailure = 3,
};

struct TransportError {
  TransportErrorCode code = TransportErrorCode::kNone;
  std::string message;

  [[nodiscard]] auto HasError() const -> bool {
    return code != TransportErrorCode::kNone;
  }
};

[[nodiscard]] auto MakeTransportError(TransportErrorCode code,
                                      std::string_view message)
    -> TransportError;

}  // namespace tracer::transport

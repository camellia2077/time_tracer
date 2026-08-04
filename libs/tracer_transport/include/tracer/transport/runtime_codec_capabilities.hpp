#pragma once

#include <string>

#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto EncodeCapabilitiesResponse(
    const CapabilitiesResponsePayload& response) -> std::string;

}  // namespace tracer::transport

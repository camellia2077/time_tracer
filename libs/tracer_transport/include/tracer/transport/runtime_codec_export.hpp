#pragma once

#include <string>

#include "tracer/transport/runtime_responses.hpp"

namespace tracer::transport {

[[nodiscard]] auto EncodeExportResponse(const ExportResponsePayload& response)
    -> std::string;

}  // namespace tracer::transport

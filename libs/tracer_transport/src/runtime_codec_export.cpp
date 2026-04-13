#include "tracer/transport/runtime_codec.hpp"

#include "nlohmann/json.hpp"

namespace tracer::transport {

auto EncodeExportResponse(const ExportResponsePayload& response)
    -> std::string {
  return nlohmann::json{
      {"ok", response.ok},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

}  // namespace tracer::transport

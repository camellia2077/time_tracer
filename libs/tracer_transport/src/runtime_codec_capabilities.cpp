#include "tracer/transport/runtime_codec_capabilities.hpp"

#include <string>

#include "nlohmann/json.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;

}  // namespace

auto EncodeCapabilitiesResponse(const CapabilitiesResponsePayload& response)
    -> std::string {
  return json{
      {"abi",
       {
           {"name", response.abi.name},
           {"version", response.abi.version},
       }},
      {"features",
       {
           {"build_info_json", response.features.build_info_json},
           {"command_contract_json", response.features.command_contract_json},
           {"runtime_log_callback", response.features.runtime_log_callback},
           {"runtime_diagnostics_callback",
            response.features.runtime_diagnostics_callback},
           {"runtime_crypto_progress_callback",
            response.features.runtime_crypto_progress_callback},
           {"runtime_ingest_json", response.features.runtime_ingest_json},
           {"runtime_ingest_sync_status_json",
            response.features.runtime_ingest_sync_status_json},
           {"runtime_convert_json", response.features.runtime_convert_json},
           {"runtime_import_json", response.features.runtime_import_json},
           {"runtime_validate_structure_json",
            response.features.runtime_validate_structure_json},
           {"runtime_validate_logic_json",
            response.features.runtime_validate_logic_json},
           {"runtime_record_activity_atomically_json",
            response.features.runtime_record_activity_atomically_json},
           {"runtime_update_activity_remark_atomically_json",
            response.features.runtime_update_activity_remark_atomically_json},
           {"runtime_config_json", response.features.runtime_config_json},
           {"runtime_query_json", response.features.runtime_query_json},
           {"runtime_temporal_insights_json",
            response.features.runtime_temporal_insights_json},
           {"runtime_insights_batch_json",
            response.features.runtime_insights_batch_json},
           {"processed_json_io", response.features.processed_json_io},
           {"insights_markdown", response.features.insights_markdown},
           {"insights_latex", response.features.insights_latex},
           {"insights_typst", response.features.insights_typst},
       }},
  }
      .dump();
}

}  // namespace tracer::transport

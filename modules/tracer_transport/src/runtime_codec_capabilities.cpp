#include "tracer/transport/runtime_codec.hpp"

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
           {"runtime_ingest_json", response.features.runtime_ingest_json},
           {"runtime_convert_json", response.features.runtime_convert_json},
           {"runtime_import_json", response.features.runtime_import_json},
           {"runtime_validate_structure_json",
            response.features.runtime_validate_structure_json},
           {"runtime_validate_logic_json",
            response.features.runtime_validate_logic_json},
           {"runtime_query_json", response.features.runtime_query_json},
           {"runtime_report_json", response.features.runtime_report_json},
           {"runtime_report_batch_json",
            response.features.runtime_report_batch_json},
           {"runtime_export_json", response.features.runtime_export_json},
           {"runtime_tree_json", response.features.runtime_tree_json},
           {"processed_json_io", response.features.processed_json_io},
           {"report_markdown", response.features.report_markdown},
           {"report_latex", response.features.report_latex},
           {"report_typst", response.features.report_typst},
       }},
  }
      .dump();
}

}  // namespace tracer::transport

#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunPipelineChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                       const fs::path& input_root) {
  RequireOk(
      api.runtime_validate_structure(
          runtime, json{{"input_path", input_root.string()}}.dump().c_str()),
      "baseline pipeline validate structure");

  RequireOk(api.runtime_validate_logic(runtime,
                                       json{{"input_path", input_root.string()},
                                            {"date_check_mode", "none"}}
                                           .dump()
                                           .c_str()),
            "baseline pipeline validate logic");

  RequireOk(
      api.runtime_convert(runtime, json{{"input_path", input_root.string()},
                                        {"date_check_mode", "none"},
                                        {"save_processed_output", false},
                                        {"validate_logic", true},
                                        {"validate_structure", true}}
                                       .dump()
                                       .c_str()),
      "baseline pipeline convert");

  RequireOk(
      api.runtime_ingest(runtime, json{{"input_path", input_root.string()},
                                       {"date_check_mode", "none"},
                                       {"save_processed_output", false}}
                                      .dump()
                                      .c_str()),
      "baseline pipeline ingest");
}

}  // namespace tracer_core_c_api_stability_internal

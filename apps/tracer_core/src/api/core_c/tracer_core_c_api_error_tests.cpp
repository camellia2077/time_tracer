// api/core_c/tracer_core_c_api_error_tests.cpp
#include "api/core_c/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunErrorPathChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                        const fs::path& converter_config) {
  TtCoreRuntimeHandle* invalid_runtime =
      api.runtime_create(nullptr, "", converter_config.string().c_str());
  Require(invalid_runtime == nullptr,
          "runtime_create should fail with empty output_root");
  const char* create_error = api.last_error();
  Require(create_error != nullptr && create_error[0] != '\0',
          "runtime_create failure should set last_error");

  RequireNotOk(api.runtime_query(runtime, "{bad json"),
               "error-injection invalid query json");
  RequireNotOk(
      api.runtime_export(runtime, json{{"format", "md"}}.dump().c_str()),
      "error-injection missing export type");
  RequireNotOk(api.runtime_tree(
                   runtime, json{{"max_depth", "bad-string"}}.dump().c_str()),
               "error-injection invalid tree max_depth");
}

}  // namespace tracer_core_c_api_stability_internal

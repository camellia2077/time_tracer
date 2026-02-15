#ifndef INFRASTRUCTURE_BOOTSTRAP_ANDROID_RUNTIME_FACTORY_H_
#define INFRASTRUCTURE_BOOTSTRAP_ANDROID_RUNTIME_FACTORY_H_

#include <filesystem>
#include <memory>

#include "application/ports/logger.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "domain/ports/diagnostics.hpp"

namespace infrastructure::bootstrap {

struct AndroidRuntimeRequest {
  std::filesystem::path db_path;
  std::filesystem::path output_root;
  std::filesystem::path converter_config_toml_path;

  std::shared_ptr<time_tracer::application::ports::ILogger> logger;
  std::shared_ptr<time_tracer::domain::ports::IDiagnosticsSink>
      diagnostics_sink;
  std::shared_ptr<time_tracer::domain::ports::IErrorReportWriter>
      error_report_writer;
};

struct AndroidRuntime {
  std::shared_ptr<ITimeTracerCoreApi> core_api;
  std::shared_ptr<void> runtime_state;
};

[[nodiscard]] auto BuildAndroidRuntime(const AndroidRuntimeRequest& request)
    -> AndroidRuntime;

}  // namespace infrastructure::bootstrap

#endif  // INFRASTRUCTURE_BOOTSTRAP_ANDROID_RUNTIME_FACTORY_H_

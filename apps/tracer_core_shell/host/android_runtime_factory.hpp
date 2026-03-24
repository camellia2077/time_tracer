// host/android_runtime_factory.hpp
#ifndef API_ANDROID_ANDROID_RUNTIME_FACTORY_H_
#define API_ANDROID_ANDROID_RUNTIME_FACTORY_H_

#include <filesystem>
#include <memory>

#include "application/runtime_bridge/logger.hpp"
#include "domain/ports/diagnostics.hpp"

namespace tracer::core::application::use_cases {

class ITracerCoreRuntime;

}  // namespace tracer::core::application::use_cases

namespace infrastructure::bootstrap {

struct AndroidRuntimeRequest {
  std::filesystem::path db_path;
  std::filesystem::path output_root;
  std::filesystem::path converter_config_toml_path;

  std::shared_ptr<tracer_core::application::runtime_bridge::ILogger> logger;
  std::shared_ptr<tracer_core::domain::ports::IDiagnosticsSink>
      diagnostics_sink;
  std::shared_ptr<tracer_core::domain::ports::IErrorReportWriter>
      error_report_writer;
};

struct AndroidRuntime {
  std::shared_ptr<tracer::core::application::use_cases::ITracerCoreRuntime>
      runtime_api;
  std::shared_ptr<void> runtime_state;
};

[[nodiscard]] auto BuildAndroidRuntime(const AndroidRuntimeRequest& request)
    -> AndroidRuntime;

}  // namespace infrastructure::bootstrap

#endif  // API_ANDROID_ANDROID_RUNTIME_FACTORY_H_

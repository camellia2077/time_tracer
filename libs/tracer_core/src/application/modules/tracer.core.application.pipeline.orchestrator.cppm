module;

#include <filesystem>
#include <memory>
#include <optional>

struct AppOptions;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IIngestInputProvider;
class IProcessedDataStorage;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

export module tracer.core.application.pipeline.orchestrator;

export import tracer.core.application.pipeline.types;

export namespace tracer::core::application::pipeline {

namespace fs = std::filesystem;

#include "application/pipeline/detail/pipeline_orchestrator_decl.inc"

}  // namespace tracer::core::application::pipeline

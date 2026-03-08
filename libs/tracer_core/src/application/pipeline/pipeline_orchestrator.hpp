#ifndef APPLICATION_PIPELINE_PIPELINE_ORCHESTRATOR_H_
#define APPLICATION_PIPELINE_PIPELINE_ORCHESTRATOR_H_

#include <filesystem>
#include <memory>
#include <optional>

#include "application/pipeline/pipeline_types.hpp"

struct AppOptions;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IIngestInputProvider;
class IProcessedDataStorage;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

namespace tracer::core::application::pipeline {

namespace fs = std::filesystem;

#include "application/pipeline/detail/pipeline_orchestrator_decl.inc"

}  // namespace tracer::core::application::pipeline

#endif  // APPLICATION_PIPELINE_PIPELINE_ORCHESTRATOR_H_

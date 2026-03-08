module;

#include <string>

namespace tracer_core::application::ports {
class IIngestInputProvider;
}  // namespace tracer_core::application::ports

export module tracer.core.application.pipeline.stages;

export import tracer.core.application.pipeline.types;

export namespace tracer::core::application::pipeline {

#include "application/pipeline/detail/pipeline_stages_decl.inc"

}  // namespace tracer::core::application::pipeline

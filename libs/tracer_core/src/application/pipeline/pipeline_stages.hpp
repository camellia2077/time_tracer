#ifndef APPLICATION_PIPELINE_PIPELINE_STAGES_H_
#define APPLICATION_PIPELINE_PIPELINE_STAGES_H_

#include <string>

#include "application/pipeline/pipeline_types.hpp"

namespace tracer_core::application::ports {
class IIngestInputProvider;
}  // namespace tracer_core::application::ports

namespace tracer::core::application::pipeline {

#include "application/pipeline/detail/pipeline_stages_decl.inc"

}  // namespace tracer::core::application::pipeline

#endif  // APPLICATION_PIPELINE_PIPELINE_STAGES_H_

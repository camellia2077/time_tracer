#ifndef APPLICATION_PIPELINE_PIPELINE_TYPES_H_
#define APPLICATION_PIPELINE_PIPELINE_TYPES_H_

#include <filesystem>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "application/dto/ingest_input_model.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/converter_config.hpp"
#include "domain/types/date_check_mode.hpp"

namespace tracer_core::application::ports {
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

namespace tracer::core::application::pipeline {

namespace fs = std::filesystem;

#include "application/pipeline/detail/pipeline_types_decl.inc"

}  // namespace tracer::core::application::pipeline

#endif  // APPLICATION_PIPELINE_PIPELINE_TYPES_H_

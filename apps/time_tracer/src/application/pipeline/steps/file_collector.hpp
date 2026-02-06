// application/pipeline/steps/file_collector.hpp
#ifndef APPLICATION_PIPELINE_STEPS_FILE_COLLECTOR_H_
#define APPLICATION_PIPELINE_STEPS_FILE_COLLECTOR_H_

#include <string>

#include "application/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class FileCollector {
 public:
  static auto Execute(PipelineContext& context,
                      const std::string& extension = ".txt") -> bool;
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_STEPS_FILE_COLLECTOR_H_

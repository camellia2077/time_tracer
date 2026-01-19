// core/pipeline/steps/file_collector.hpp
#ifndef CORE_PIPELINE_STEPS_FILE_COLLECTOR_HPP_
#define CORE_PIPELINE_STEPS_FILE_COLLECTOR_HPP_

#include <string>
#include "core/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class FileCollector {
public:
    bool execute(PipelineContext& context, const std::string& extension = ".txt");
};

} // namespace core::pipeline

#endif // CORE_PIPELINE_STEPS_FILE_COLLECTOR_HPP_
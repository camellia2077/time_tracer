// core/file/steps/FileCollector.hpp
#ifndef CORE_FILE_STEPS_FILE_COLLECTOR_HPP_
#define CORE_FILE_STEPS_FILE_COLLECTOR_HPP_

#include <string>
#include "core/file/PipelineContext.hpp"

class FileCollector {
public:
    bool execute(PipelineContext& context, const std::string& extension = ".txt");
};

#endif // CORE_FILE_STEPS_FILE_COLLECTOR_HPP_
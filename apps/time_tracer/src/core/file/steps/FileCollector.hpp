// core/file/steps/FileCollector.hpp
#ifndef FILE_COLLECTOR_HPP
#define FILE_COLLECTOR_HPP

#include <string>
#include "core/file/PipelineContext.hpp"

class FileCollector {
public:
    bool execute(PipelineContext& context, const std::string& extension = ".txt");
};

#endif // FILE_COLLECTOR_HPP
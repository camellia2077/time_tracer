#ifndef FILE_COLLECTOR_HPP
#define FILE_COLLECTOR_HPP

#include <string>
#include "action_handler/file/PipelineContext.hpp"

class FileCollector {
public:
    bool execute(PipelineContext& context, const std::string& extension = ".txt");
};

#endif // FILE_COLLECTOR_HPP
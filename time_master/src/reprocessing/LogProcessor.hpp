// reprocessing/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp" // 引入新的通用配置头文件

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    ProcessingResult processFile(const std::filesystem::path& source_file, 
                                 const std::filesystem::path& output_file, 
                                 const AppOptions& options);
    
    bool collectFilesToProcess(const std::string& input_path, std::vector<std::filesystem::path>& out_files);


private:
    AppConfig config_;

};

#endif // LOG_PROCESSOR_HPP
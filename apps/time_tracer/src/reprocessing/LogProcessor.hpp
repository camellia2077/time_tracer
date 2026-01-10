// reprocessing/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp"
#include "reprocessing/converter/model/InputData.hpp" 

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    // 纯业务逻辑：将流转换为数据对象
    std::vector<InputData> convertStreamToData(std::istream& combined_stream);

    // 纯业务逻辑：验证单个文件（虽然它打开了文件，但主要关注验证规则）
    ProcessingResult processFile(const std::filesystem::path& source_file, 
                                 const AppOptions& options);
    
    // [删除] 文件查找职责不属于 Processor，已由 FileCollector/FileUtils 接管
    // bool collectFilesToProcess(const std::filesystem::path& input_path, std::vector<std::filesystem::path>& out_files);

private:
    AppConfig config_;
};

#endif // LOG_PROCESSOR_HPP
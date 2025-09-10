// reprocessing/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp"
#include "reprocessing/converter/model/InputData.hpp" // [核心修改]

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    // --- [核心修改] ---
    // 这个方法现在只负责调用转换器并返回内存中的数据
    std::vector<InputData> convertStreamToData(std::istream& combined_stream);

    // --- [核心修改] ---
    // 保持 processFile 接口，但其实现将被简化，不再写入文件
    ProcessingResult processFile(const std::filesystem::path& source_file, 
                                 const AppOptions& options);
    
    bool collectFilesToProcess(const std::string& input_path, std::vector<std::filesystem::path>& out_files);


private:
    AppConfig config_;
};

#endif // LOG_PROCESSOR_HPP
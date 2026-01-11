// converter/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <istream> 
#include "common/AppConfig.hpp" // ProcessingResult 已经在此头文件中定义
#include "converter/convert/model/InputData.hpp" 
#include "converter/convert/config/ConverterConfig.hpp"

class LogProcessor {
public:
    // 接收已加载的 ConverterConfig
    explicit LogProcessor(const ConverterConfig& config);

    std::vector<InputData> convertStreamToData(std::istream& combined_stream);

    // 接收内容字符串进行验证
    ProcessingResult processSourceContent(const std::string& filename, 
                                          const std::string& content);

private:
    const ConverterConfig& config_;
};

#endif // LOG_PROCESSOR_HPP
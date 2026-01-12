// converter/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <istream> 
#include <functional> 
#include "common/AppConfig.hpp"
#include "converter/convert/model/InputData.hpp" 
#include "converter/convert/config/ConverterConfig.hpp"

class LogProcessor {
public:
    explicit LogProcessor(const ConverterConfig& config);

    /**
     * @brief 将输入流转换为数据对象
     * * [I/O 边界说明]
     * 此函数不执行磁盘写入。它将转换后的 InputData 通过 data_consumer 回调传出。
     * 调用者 (Core 层) 负责在回调中实现具体的存储逻辑 (例如调用 Output 类生成 JSON 并写入文件)。
     */
    void convertStreamToData(std::istream& combined_stream, std::function<void(InputData&&)> data_consumer);

    ProcessingResult processSourceContent(const std::string& filename, 
                                          const std::string& content);

private:
    const ConverterConfig& config_;
};

#endif // LOG_PROCESSOR_HPP
// converter/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <istream> 
#include <functional>
#include <map> 
// [修复] 移除 AppConfig 依赖，解耦 Lib 和 App
// #include "common/AppConfig.hpp" 
#include "common/model/DailyLog.hpp" 
#include "converter/config/ConverterConfig.hpp"

// [修复] 定义专门的返回结果结构体，包含数据
struct LogProcessingResult {
    bool success = true;
    // 包含转换后的数据：Key=YYYY-MM, Value=List of Logs
    std::map<std::string, std::vector<DailyLog>> processed_data;
};

class LogProcessor {
public:
    explicit LogProcessor(const ConverterConfig& config);

    /**
     * @brief 将输入流转换为数据对象
     * * [I/O 边界说明]
     * 此函数不执行磁盘写入。它将转换后的 DailyLog 通过 data_consumer 回调传出。
     * 调用者 (Core 层) 负责在回调中实现具体的存储逻辑 (例如调用 Output 类生成 JSON 并写入文件)。
     */
    void convertStreamToData(std::istream& combined_stream, std::function<void(DailyLog&&)> data_consumer);

    // [修复] 返回类型改为 LogProcessingResult
    LogProcessingResult processSourceContent(const std::string& filename, 
                                             const std::string& content);

private:
    const ConverterConfig& config_;
};

#endif // LOG_PROCESSOR_HPP
// converter/LogProcessor.hpp
#ifndef CONVERTER_LOG_PROCESSOR_HPP_
#define CONVERTER_LOG_PROCESSOR_HPP_

#include <string>
#include <vector>
#include <istream> 
#include <functional>
#include <map> 
#include "common/model/DailyLog.hpp" 
#include "converter/config/ConverterConfig.hpp"

struct LogProcessingResult {
    bool success = true;
    std::map<std::string, std::vector<DailyLog>> processed_data;
};

class LogProcessor {
public:
    explicit LogProcessor(const ConverterConfig& config);

    /**
     * @brief 将输入流转换为数据对象
     */
    void convertStreamToData(std::istream& combined_stream, std::function<void(DailyLog&&)> data_consumer);

    /**
     * @brief 处理源内容
     * [修改] 现在此函数不再执行验证，仅执行转换。
     * 验证步骤已移至 Core 层的 SourceValidatorStep 中独立执行。
     */
    LogProcessingResult processSourceContent(const std::string& filename, 
                                             const std::string& content);

private:
    const ConverterConfig& config_;
};

#endif // CONVERTER_LOG_PROCESSOR_HPP_
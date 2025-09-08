// reprocessing/converter/IntervalConverter.hpp
#ifndef INTERVAL_PROCESSOR_HPP
#define INTERVAL_PROCESSOR_HPP


#include "reprocessing/converter/internal/ConverterConfig.hpp"
#include "common/AppConfig.hpp" // [新增] 引入 AppConfig

#include <string>

class IntervalConverter {
public:
    // [修改] 构造函数接收 AppConfig
    explicit IntervalConverter(const std::string& config_filename, const AppConfig& app_config);
    bool executeConversion(const std::string& input_filepath, const std::string& output_filepath);

private:
    ConverterConfig config_;
};

#endif // INTERVAL_PROCESSOR_HPP
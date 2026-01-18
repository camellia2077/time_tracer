// core/file/steps/converter_step.cpp
#include "converter_step.hpp"
#include "core/file/utils/converter_config_factory.hpp" 
#include "common/ansi_colors.hpp"
#include "io/core/file_reader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

ConverterStep::ConverterStep(const AppConfig& config) : app_config_(config) {}

bool ConverterStep::execute(PipelineContext& context) {
    std::cout << "Step: Converting files (Memory)..." << std::endl;
    auto start_time = std::chrono::steady_clock::now();

    // 1. 加载配置
    try {
        context.state.converter_config = ConverterConfigFactory::create(
            app_config_.pipeline.interval_processor_config_path,
            app_config_
        );
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "配置加载严重错误: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }

    LogProcessor processor(context.state.converter_config);
    bool all_success = true;

    // 2. 执行转换逻辑 (Text -> Struct)
    for (const auto& file_path : context.state.source_files) {
        try {
            std::string content = FileReader::read_content(file_path);
            LogProcessingResult result = processor.processSourceContent(file_path.string(), content);
            
            if (!result.success) {
                std::cerr << RED_COLOR << "处理失败: " << file_path << RESET_COLOR << std::endl;
                all_success = false;
                continue;
            }

            // 将生成的结构体合并到上下文中
            context.result.processed_data.insert(result.processed_data.begin(), result.processed_data.end());

        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "转换异常: " << file_path << " - " << e.what() << RESET_COLOR << std::endl;
             all_success = false;
        }
    }

    // [移除] 移除数据持久化逻辑，推迟到 OutputValidator 之后
    
    auto end_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    printTiming(duration);
    
    if (all_success) {
        std::cout << GREEN_COLOR << "内存转换阶段 全部成功。" << RESET_COLOR << std::endl;
    } else {
        std::cout << YELLOW_COLOR << "内存转换阶段 完成，但存在部分错误。" << RESET_COLOR << std::endl;
    }

    return all_success;
}

void ConverterStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "转换耗时: " << std::fixed << std::setprecision(3) << total_time_s << " 秒 (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}
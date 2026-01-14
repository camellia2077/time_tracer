// core/file/steps/ConverterStep.cpp
#include "ConverterStep.hpp"
#include "core/file/utils/ConverterConfigFactory.hpp" // [Refactor] 使用工厂
#include "core/file/utils/ProcessedDataWriter.hpp"    // [Refactor] 使用写入器
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

ConverterStep::ConverterStep(const AppConfig& config) : app_config_(config) {}

bool ConverterStep::execute(PipelineContext& context) {
    std::cout << "Step: Converting files..." << std::endl;
    auto start_time = std::chrono::steady_clock::now();

    // 1. 加载配置 (Delegated to Factory)
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

    // 2. 执行转换逻辑
    for (const auto& file_path : context.state.source_files) {
        try {
            std::string content = FileReader::read_content(file_path);
            LogProcessingResult result = processor.processSourceContent(file_path.string(), content);
            
            if (!result.success) {
                std::cerr << RED_COLOR << "处理失败: " << file_path << RESET_COLOR << std::endl;
                all_success = false;
                continue;
            }

            // 将结果合并到上下文中
            context.result.processed_data.insert(result.processed_data.begin(), result.processed_data.end());

        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "转换异常: " << file_path << " - " << e.what() << RESET_COLOR << std::endl;
             all_success = false;
        }
    }

    // 3. 数据持久化 (Delegated to Writer)
    if (context.config.save_processed_output) {
        auto new_files = ProcessedDataWriter::write(
            context.result.processed_data,
            context.config.output_root,
            context.state.converter_config
        );
        
        // 更新生成文件列表
        context.state.generated_files.insert(
            context.state.generated_files.end(),
            new_files.begin(), 
            new_files.end()
        );

        if (new_files.empty() && !context.result.processed_data.empty()) {
            // 如果有数据但没生成文件，说明写入过程全失败了
             all_success = false; 
        }
    } else {
        std::cout << YELLOW_COLOR << "信息: 转换后的 JSON 数据驻留内存，未写入磁盘。" << RESET_COLOR << std::endl;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    printTiming(duration);
    
    if (all_success) {
        std::cout << GREEN_COLOR << "文件转换阶段 全部成功。" << RESET_COLOR << std::endl;
    } else {
        std::cout << YELLOW_COLOR << "文件转换阶段 完成，但存在部分错误。" << RESET_COLOR << std::endl;
    }

    return all_success;
}

void ConverterStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "转换耗时: " << std::fixed << std::setprecision(3) << total_time_s << " 秒 (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}
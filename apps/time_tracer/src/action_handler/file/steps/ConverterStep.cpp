// action_handler/file/steps/ConverterStep.cpp
#include "ConverterStep.hpp"
#include "common/AnsiColors.hpp"
#include "reprocessing/converter/pipelines/Output.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <chrono>
#include <iomanip>

ConverterStep::ConverterStep(const AppConfig& config) : processor_(config) {}

bool ConverterStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 转换文件 ---" << std::endl;
    
    if (context.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供转换。" << RESET_COLOR << std::endl;
        return false;
    }

    auto start_time = std::chrono::steady_clock::now();

    // 1. 合并文件流
    std::stringstream combined_stream;
    for (const auto& source_file : context.source_files) {
        std::ifstream file_stream(source_file);
        if (file_stream) {
            combined_stream << file_stream.rdbuf();
        }
    }
    
    // 2. 执行转换
    std::vector<InputData> all_processed_days = processor_.convertStreamToData(combined_stream);
    if (all_processed_days.empty()) {
        std::cerr << RED_COLOR << "文件转换失败，没有生成任何数据。" << RESET_COLOR << std::endl;
        return false;
    }

    // 3. 按月分组 [核心修改]
    std::map<std::string, std::vector<InputData>> monthly_data;
    for (const auto& day : all_processed_days) {
        // 适配 YYYY-MM-DD 格式 (长度 10)
        if (day.date.length() == 10) {
            // day.date 是 "2025-01-01"
            // substr(0, 4) -> "2025"
            // substr(5, 2) -> "01" (跳过中间的 '-')
            std::string year_month = day.date.substr(0, 4) + "_" + day.date.substr(5, 2);
            monthly_data[year_month].push_back(day);
        }
    }

    // [核心修改] 将数据保存到 Context 中 (无论是否写入磁盘)
    context.processed_data_memory = monthly_data;

    // 4. 加载转换配置
    ConverterConfig converter_config;
    if (!converter_config.load(context.config.interval_processor_config_path)) {
        std::cerr << RED_COLOR << "错误: 无法加载转换器配置。" << RESET_COLOR << std::endl;
        return false;
    }
    
    // 5. 写入输出文件 (仅当用户要求时)
    if (context.save_processed_output) {
        Output generator;
        fs::path output_dir_base = context.output_root / "Processed_Date";

        for (const auto& pair : monthly_data) {
            const std::string& year_month = pair.first;
            const std::vector<InputData>& month_days = pair.second;
            
            std::string year = year_month.substr(0, 4);
            fs::path month_output_dir = output_dir_base / year;
            fs::create_directories(month_output_dir);

            fs::path output_file_path = month_output_dir / (year_month + ".json");
            std::ofstream outFile(output_file_path);
            if (!outFile.is_open()) {
                std::cerr << RED_COLOR << "错误: 无法创建输出文件: " << output_file_path << RESET_COLOR << std::endl;
                continue;
            }
            
            generator.write(outFile, month_days, converter_config); 
            context.generated_files.push_back(output_file_path);
        }
    } else {
        std::cout << YELLOW_COLOR << "信息: 转换后的 JSON 数据驻留内存，未写入磁盘。" << RESET_COLOR << std::endl;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    printTiming(duration);
    
    std::cout << GREEN_COLOR << "文件转换阶段 全部成功。" << RESET_COLOR << std::endl;
    return true;
}

void ConverterStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "Timing Statistics for: convertFiles\n\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(4) << total_time_s
              << " seconds (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}
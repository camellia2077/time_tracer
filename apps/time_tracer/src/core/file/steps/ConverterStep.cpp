// core/file/steps/ConverterStep.cpp
#include "ConverterStep.hpp"
#include "common/AnsiColors.hpp"
#include "converter/convert/io/JsonWriter.hpp"
#include "io/core/FileReader.hpp"      
#include "io/core/FileWriter.hpp"      
#include "io/core/FileSystemHelper.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

// 辅助函数
static ConverterConfig loadAndAssembleConfig(const fs::path& main_config_path) {
    using json = nlohmann::json;
    ConverterConfig config;
    try {
        std::string main_content = FileReader::read_content(main_config_path);
        json main_json = json::parse(main_content);
        fs::path config_dir = main_config_path.parent_path();

        if (main_json.contains("mappings_config_path")) {
            fs::path map_path = config_dir / main_json["mappings_config_path"].get<std::string>();
            std::string map_content = FileReader::read_content(map_path);
            json map_json = json::parse(map_content);
            if (map_json.contains("text_mappings")) {
                main_json["text_mappings"] = map_json["text_mappings"];
            }
        }
        
        // [修复] 正确加载 duration_rules.json 中的字段
        if (main_json.contains("duration_rules_config_path")) {
            fs::path dur_path = config_dir / main_json["duration_rules_config_path"].get<std::string>();
            std::string dur_content = FileReader::read_content(dur_path);
            json dur_json = json::parse(dur_content);
            
            // 修复：分别合并 text_duration_mappings 和 duration_mappings
            if (dur_json.contains("text_duration_mappings")) {
                main_json["text_duration_mappings"] = dur_json["text_duration_mappings"];
            }
            if (dur_json.contains("duration_mappings")) {
                main_json["duration_mappings"] = dur_json["duration_mappings"];
            }
        }

        config.load(main_json);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error loading converter config: " << e.what() << RESET_COLOR << std::endl;
    }
    return config;
}

ConverterStep::ConverterStep(const AppConfig& config) : app_config_(config) {}

bool ConverterStep::execute(PipelineContext& context) {
    // ... (后续代码保持不变) ...
    std::cout << "Step: Converting files..." << std::endl;
    auto start_time = std::chrono::steady_clock::now();

    context.state.converter_config = loadAndAssembleConfig(app_config_.pipeline.interval_processor_config_path);
    
    // 手动转换 path map 到 string map
    for (const auto& [path_key, path_val] : app_config_.pipeline.initial_top_parents) {
        context.state.converter_config.initial_top_parents[path_key.string()] = path_val.string();
    }

    LogProcessor processor(context.state.converter_config);
    bool all_success = true;

    // 处理每个源文件
    for (const auto& file_path : context.state.source_files) {
        try {
            std::string content = FileReader::read_content(file_path);
            LogProcessingResult result = processor.processSourceContent(file_path.string(), content);
            
            if (!result.success) {
                std::cerr << RED_COLOR << "处理失败: " << file_path << RESET_COLOR << std::endl;
                all_success = false;
                continue;
            }

            // 存入内存结果集
            context.result.processed_data.insert(result.processed_data.begin(), result.processed_data.end());

        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "错误: 处理文件时发生异常: " << file_path << " - " << e.what() << RESET_COLOR << std::endl;
             all_success = false;
        }
    }

    // 如果配置要求保存到磁盘
    if (context.config.save_processed_output) {
         JsonWriter generator;
         for (const auto& [month_key, month_days] : context.result.processed_data) {
            fs::path output_file_path = context.config.output_root / "Processed_Date" / (month_key + ".json");
            fs::path month_output_dir = output_file_path.parent_path();

            try {
                FileSystemHelper::create_directories(month_output_dir);
                std::stringstream buffer;
                generator.write(buffer, month_days, context.state.converter_config); 
                FileWriter::write_content(output_file_path, buffer.str());
                
                context.state.generated_files.push_back(output_file_path);
            } catch (const std::exception& e) {
                 std::cerr << RED_COLOR << "错误: 无法写入输出文件: " << output_file_path << " - " << e.what() << RESET_COLOR << std::endl;
                 all_success = false; 
                 continue;
            }
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
// core/file/steps/ConverterStep.cpp
#include "ConverterStep.hpp"
#include "common/AnsiColors.hpp"
#include "converter/convert/pipelines/Output.hpp"
#include "io/core/FileReader.hpp"      
#include "io/core/FileWriter.hpp"      
#include "io/core/FileSystemHelper.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

// 辅助函数保持不变
static ConverterConfig loadAndAssembleConfig(const fs::path& main_config_path) {
    // ... (代码同前，略)
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
        if (main_json.contains("duration_rules_config_path")) {
            fs::path dur_path = config_dir / main_json["duration_rules_config_path"].get<std::string>();
            std::string dur_content = FileReader::read_content(dur_path);
            json dur_json = json::parse(dur_content);
            if (dur_json.contains("text_duration_mappings")) {
                main_json["text_duration_mappings"] = dur_json["text_duration_mappings"];
            }
            if (dur_json.contains("duration_mappings")) {
                main_json["duration_mappings"] = dur_json["duration_mappings"];
            }
        }
        config.load(main_json);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error assembling converter config: " << e.what() << RESET_COLOR << std::endl;
    }
    return config;
}

ConverterStep::ConverterStep(const AppConfig& config) : app_config_(config) {}

bool ConverterStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 转换文件 ---" << std::endl;
    
    // [修正] source_files -> state.source_files
    if (context.state.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供转换。" << RESET_COLOR << std::endl;
        return false;
    }

    auto start_time = std::chrono::steady_clock::now();

    // 1. 准备配置
    // [修正] converter_config -> state.converter_config
    context.state.converter_config = loadAndAssembleConfig(app_config_.interval_processor_config_path);
    LogProcessor processor(context.state.converter_config);

    // 2. 合并文件流
    std::stringstream combined_stream;
    for (const auto& source_file : context.state.source_files) {
        try {
            std::string content = FileReader::read_content(source_file);
            combined_stream << content;
        } catch (const std::exception& e) {
            std::cerr << YELLOW_COLOR << "警告: 读取文件失败 " << source_file << ": " << e.what() << RESET_COLOR << std::endl;
        }
    }
    
    // 3. 执行转换
    std::vector<InputData> all_processed_days = processor.convertStreamToData(combined_stream);
    
    if (all_processed_days.empty()) {
        std::cerr << RED_COLOR << "文件转换失败，没有生成任何数据。" << RESET_COLOR << std::endl;
        return false;
    }

    // 4. 按月分组
    std::map<std::string, std::vector<InputData>> monthly_data;
    for (const auto& day : all_processed_days) {
        if (day.date.length() == 10) {
            std::string year_month = day.date.substr(0, 4) + "_" + day.date.substr(5, 2);
            monthly_data[year_month].push_back(day);
        }
    }

    // [修正] processed_data_memory -> result.processed_data
    context.result.processed_data = monthly_data;

    // 5. 写入输出
    // [修正] save_processed_output -> config.save_processed_output
    if (context.config.save_processed_output) {
        Output generator;
        // [修正] output_root -> config.output_root
        fs::path output_dir_base = context.config.output_root / "Processed_Date";

        for (const auto& pair : monthly_data) {
            const std::string& year_month = pair.first;
            const std::vector<InputData>& month_days = pair.second;
            
            std::string year = year_month.substr(0, 4);
            fs::path month_output_dir = output_dir_base / year;
            fs::path output_file_path = month_output_dir / (year_month + ".json");

            try {
                FileSystemHelper::create_directories(month_output_dir);
                std::stringstream buffer;
                // [修正] converter_config -> state.converter_config
                generator.write(buffer, month_days, context.state.converter_config); 
                FileWriter::write_content(output_file_path, buffer.str());
                
                // [修正] generated_files -> state.generated_files
                context.state.generated_files.push_back(output_file_path);
            } catch (const std::exception& e) {
                 std::cerr << RED_COLOR << "错误: 无法写入输出文件: " << output_file_path << " - " << e.what() << RESET_COLOR << std::endl;
                 continue;
            }
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
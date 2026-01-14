// core/file/steps/SourceValidatorStep.cpp
#include "SourceValidatorStep.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

// 静态辅助函数
static ConverterConfig loadConfig(const fs::path& main_config_path) {
    using json = nlohmann::json;
    ConverterConfig config;
    try {
        std::string main_content = FileReader::read_content(main_config_path);
        json main_json = json::parse(main_content);
        fs::path config_dir = main_config_path.parent_path();

        if (main_json.contains("mappings_config_path")) {
            fs::path map_path = config_dir / main_json["mappings_config_path"].get<std::string>();
            // [调试信息] 确认是否尝试加载映射文件
            std::cout << "[Validator] Loading mappings from: " << map_path << std::endl;
            
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
        std::cerr << RED_COLOR << "Config Error in SourceValidatorStep: " << e.what() << RESET_COLOR << std::endl;
    }
    return config;
}

bool SourceValidatorStep::execute(PipelineContext& context) {
    // ... (后续代码保持不变) ...
    std::cout << "Step: Validating source files..." << std::endl;

    ConverterConfig config = loadConfig(context.config.app_config.pipeline.interval_processor_config_path);
    
    // [核心修复] 手动注入 initial_top_parents
    for (const auto& [path_key, path_val] : context.config.app_config.pipeline.initial_top_parents) {
        config.initial_top_parents[path_key.string()] = path_val.string();
    }
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    LogProcessor processor(config);

    for (const auto& file_path : context.state.source_files) {
        auto start_time = std::chrono::steady_clock::now();

        try {
            std::string content = FileReader::read_content(file_path);
            LogProcessingResult result = processor.processSourceContent(file_path.string(), content);
            if (!result.success) all_ok = false;
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "IO Error validating " << file_path << ": " << e.what() << RESET_COLOR << std::endl;
            all_ok = false;
        }
        
        auto end_time = std::chrono::steady_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    printTiming(total_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "源文件检验阶段 " 
              << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

void SourceValidatorStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "检验耗时: " << std::fixed << std::setprecision(3) << total_time_s << " 秒 (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}
// core/file/steps/SourceValidatorStep.cpp
#include "SourceValidatorStep.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

// 静态辅助函数保持不变...
static ConverterConfig loadConfig(const fs::path& main_config_path) {
    // ... (同原代码)
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
            if (map_json.contains("text_mappings")) main_json["text_mappings"] = map_json["text_mappings"];
        }
        
        if (main_json.contains("duration_rules_config_path")) {
            fs::path dur_path = config_dir / main_json["duration_rules_config_path"].get<std::string>();
            std::string dur_content = FileReader::read_content(dur_path);
            json dur_json = json::parse(dur_content);
            if (dur_json.contains("text_duration_mappings")) main_json["text_duration_mappings"] = dur_json["text_duration_mappings"];
            if (dur_json.contains("duration_mappings")) main_json["duration_mappings"] = dur_json["duration_mappings"];
        }

        config.load(main_json);
    } catch (...) {}
    return config;
}

bool SourceValidatorStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 检验源文件 ---" << std::endl;
    
    // [修正] context.source_files -> context.state.source_files
    if (context.state.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }

    // [修正] context.config 现在是 PipelineConfig，其中的 app_config 持有原始 AppConfig
    ConverterConfig config = loadConfig(context.config.app_config.interval_processor_config_path);
    
    // [修正] context.converter_config -> context.state.converter_config
    context.state.converter_config = config;

    bool all_ok = true;
    double total_ms = 0.0;
    
    LogProcessor processor(config);

    for (const auto& file_path : context.state.source_files) {
        auto start_time = std::chrono::steady_clock::now();

        try {
            std::string content = FileReader::read_content(file_path);
            ProcessingResult result = processor.processSourceContent(file_path.string(), content);
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
    std::cout << "Timing Statistics for: validateSourceFiles\n\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(4) << total_time_s
              << " seconds (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}
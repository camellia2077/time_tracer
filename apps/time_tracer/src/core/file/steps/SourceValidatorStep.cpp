// core/file/steps/SourceValidatorStep.cpp
#include "SourceValidatorStep.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp" // [New]
#include <iostream>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

// 复制 Config 组装逻辑，或将其放入 Context 中 (如果 ConverterStep 先运行)
// 假设 Pipeline 顺序不确定，这里我们简单地再次加载。
// 在实际优化中，应该将 loadAndAssembleConfig 提取到 Utils 或 Context 初始化中。
static ConverterConfig loadConfig(const fs::path& main_config_path) {
    // 简化的副本：实际建议放在 common helper 中
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
    
    if (context.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }

    // 确保 Context 中有 config，如果没有则加载
    // 这里我们先重新加载一份本地的
    ConverterConfig config = loadConfig(context.config.interval_processor_config_path);
    // 也可以赋值给 context.converter_config 供后续使用
    context.converter_config = config;

    bool all_ok = true;
    double total_ms = 0.0;
    
    LogProcessor processor(config);

    for (const auto& file_path : context.source_files) {
        auto start_time = std::chrono::steady_clock::now();

        // [核心修改] 使用 IO 读取内容，传给 Processor
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
#include "SourceValidatorStep.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

bool SourceValidatorStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 检验源文件 ---" << std::endl;
    
    if (context.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    double total_ms = 0.0;
    
    // 创建临时的 Processor 用于验证
    LogProcessor processor(context.config);

    for (const auto& file : context.source_files) {
        auto start_time = std::chrono::steady_clock::now();

        AppOptions opts;
        opts.validate_source = true;
        
        ProcessingResult result = processor.processFile(file, opts);
        
        auto end_time = std::chrono::steady_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();

        if (!result.success) all_ok = false;
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
// core/file/steps/SourceValidatorStep.cpp
#include "SourceValidatorStep.hpp"
#include "core/file/utils/ConverterConfigFactory.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

// [修改] 引用新的头文件
#include "validator/txt/facade/TextValidator.hpp" 

// [新增] 命名空间引用
using namespace validator;
using namespace validator::txt;

bool SourceValidatorStep::execute(PipelineContext& context) {
    std::cout << "Step: Validating source files..." << std::endl;

    ConverterConfig config;
    try {
        config = ConverterConfigFactory::create(
            context.config.app_config.pipeline.interval_processor_config_path,
            context.config.app_config
        );
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "验证准备阶段配置加载失败: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    // [修改] 类在 namespace validator::txt 中
    TextValidator validator(config);

    for (const auto& file_path : context.state.source_files) {
        auto start_time = std::chrono::steady_clock::now();
        std::set<Error> errors; // [修改] 在 namespace validator 中

        try {
            std::string content = FileReader::read_content(file_path);
            
            if (!validator.validate(file_path.string(), content, errors)) {
                printGroupedErrors(file_path.string(), errors); // [修改] 在 namespace validator 中
                all_ok = false;
            }
            
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
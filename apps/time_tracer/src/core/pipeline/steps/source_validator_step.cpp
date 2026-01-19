// core/pipeline/steps/source_validator_step.cpp
#include "source_validator_step.hpp"
#include "common/ansi_colors.hpp"
#include "io/core/file_reader.hpp"
#include "validator/txt/facade/TextValidator.hpp" 
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace validator;
using namespace validator::txt;

namespace core::pipeline {

bool SourceValidatorStep::execute(PipelineContext& context) {
    std::cout << "Step: Validating source files..." << std::endl;
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    // TextValidator 构造函数现在接收 const ConverterConfig& (Struct)
    TextValidator validator(context.state.converter_config);

    for (const auto& file_path : context.state.source_files) {
        auto start_time = std::chrono::steady_clock::now();
        std::set<Error> errors;

        try {
            std::string content = FileReader::read_content(file_path);
            
            if (!validator.validate(file_path.string(), content, errors)) {
                printGroupedErrors(file_path.string(), errors);
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

} // namespace core::pipeline
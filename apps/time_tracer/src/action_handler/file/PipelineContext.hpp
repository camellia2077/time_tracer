// action_handler/file/PipelineContext.hpp
#ifndef PIPELINE_CONTEXT_HPP
#define PIPELINE_CONTEXT_HPP

#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

struct PipelineContext {
    // 基础配置
    const AppConfig& config;
    
    // 输入输出路径
    fs::path input_root;
    fs::path output_root;
    
    // 运行时状态 - 在步骤间共享
    std::vector<fs::path> source_files;      // FileCollector 填充
    std::vector<fs::path> generated_files;   // ConverterStep 填充
    
    // 验证模式参数
    DateCheckMode date_check_mode = DateCheckMode::None;
    
    explicit PipelineContext(const AppConfig& cfg, const fs::path& out_root) 
        : config(cfg), output_root(out_root) {}
};

#endif // PIPELINE_CONTEXT_HPP
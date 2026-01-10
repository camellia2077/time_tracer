// action_handler/file/PipelineContext.hpp
#ifndef PIPELINE_CONTEXT_HPP
#define PIPELINE_CONTEXT_HPP

#include <vector>
#include <filesystem>
#include <map>
#include "common/AppConfig.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/model/InputData.hpp" // [新增] 引入数据模型

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
    
    // [新增] 内存中的转换数据：Key为 "YYYY_MM"，Value为该月的数据列表
    // 这使得 run-pipeline 可以跳过磁盘读写，直接将数据传给 DBManager
    std::map<std::string, std::vector<InputData>> processed_data_memory;
    
    // 验证模式参数
    DateCheckMode date_check_mode = DateCheckMode::None;

    // 是否保存转换后的文件到磁盘
    bool save_processed_output = false;
    
    explicit PipelineContext(const AppConfig& cfg, const fs::path& out_root) 
        : config(cfg), output_root(out_root) {}
};

#endif // PIPELINE_CONTEXT_HPP
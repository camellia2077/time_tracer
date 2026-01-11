// core/file/PipelineContext.hpp
#ifndef PIPELINE_CONTEXT_HPP
#define PIPELINE_CONTEXT_HPP

#include <vector>
#include <filesystem>
#include <map>
#include "common/AppConfig.hpp"
#include "converter/validator/common/ValidatorUtils.hpp"
#include "converter/convert/model/InputData.hpp"
#include "converter/convert/config/ConverterConfig.hpp" // [新增]

namespace fs = std::filesystem;

struct PipelineContext {
    const AppConfig& config;
    
    fs::path input_root;
    fs::path output_root;
    
    std::vector<fs::path> source_files;
    std::vector<fs::path> generated_files;
    
    std::map<std::string, std::vector<InputData>> processed_data_memory;
    
    // [新增] 缓存加载后的 ConverterConfig，供各步骤共享
    ConverterConfig converter_config;

    DateCheckMode date_check_mode = DateCheckMode::None;
    bool save_processed_output = false;
    
    explicit PipelineContext(const AppConfig& cfg, const fs::path& out_root) 
        : config(cfg), output_root(out_root) {}
};

#endif // PIPELINE_CONTEXT_HPP
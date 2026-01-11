// core/file/PipelineContext.hpp
#ifndef PIPELINE_CONTEXT_HPP
#define PIPELINE_CONTEXT_HPP

#include <vector>
#include <filesystem>
#include <map>
#include "common/AppConfig.hpp"
#include "converter/validator/common/ValidatorUtils.hpp"
#include "converter/convert/model/InputData.hpp"
#include "converter/convert/config/ConverterConfig.hpp"

namespace fs = std::filesystem;

/**
 * @brief Pipeline 任务的静态配置 (Read-Only)
 * 初始化后不应修改
 */
struct PipelineConfig {
    const AppConfig& app_config;
    
    fs::path input_root;
    fs::path output_root;
    
    DateCheckMode date_check_mode = DateCheckMode::None;
    bool save_processed_output = false;

    PipelineConfig(const AppConfig& cfg, fs::path out) 
        : app_config(cfg), output_root(std::move(out)) {}
};

/**
 * @brief Pipeline 运行时的可变状态 (Mutable State)
 * 包含中间文件列表和运行时加载的配置
 */
struct PipelineState {
    std::vector<fs::path> source_files;
    std::vector<fs::path> generated_files;
    
    // 运行时加载的 ConverterConfig，供 ConverterStep 和 Validator 使用
    ConverterConfig converter_config;
};

/**
 * @brief Pipeline 的业务数据产出 (Result)
 * 专门存放转换后的内存数据，与文件状态分离
 */
struct PipelineResult {
    // 转换后的核心业务数据
    std::map<std::string, std::vector<InputData>> processed_data;

    bool has_data() const {
        return !processed_data.empty();
    }
};

/**
 * @brief Pipeline 上下文容器
 * 聚合配置、状态和结果，作为 Step 间传递信息的载体
 */
struct PipelineContext {
    PipelineConfig config;
    PipelineState state;
    PipelineResult result;
    
    explicit PipelineContext(const AppConfig& cfg, const fs::path& out_root) 
        : config(cfg, out_root) {}
};

#endif // PIPELINE_CONTEXT_HPP
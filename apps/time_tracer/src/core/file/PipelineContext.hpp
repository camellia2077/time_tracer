// core/file/PipelineContext.hpp
#ifndef PIPELINE_CONTEXT_HPP
#define PIPELINE_CONTEXT_HPP

#include <vector>
#include <filesystem>
#include <map>
#include "common/AppConfig.hpp"
#include "converter/validator/common/ValidatorUtils.hpp"
#include "common/model/DailyLog.hpp"
#include "converter/config/ConverterConfig.hpp"


namespace fs = std::filesystem;

/**
 * @brief Pipeline 任务的运行时配置 (Runtime Config)
 * [修复] 重命名为 PipelineRunConfig 以避免与 AppConfig.hpp 中的 PipelineConfig 冲突
 * 包含本次运行特有的路径（如 input/output root）和全局配置的引用
 */
struct PipelineRunConfig {
    const AppConfig& app_config;
    
    fs::path input_root;
    fs::path output_root;
    
    DateCheckMode date_check_mode = DateCheckMode::None;
    bool save_processed_output = false;

    PipelineRunConfig(const AppConfig& cfg, fs::path out) 
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
    std::map<std::string, std::vector<DailyLog>> processed_data;
};

/**
 * @brief Pipeline 上下文
 * 贯穿整个处理流程，持有配置、状态和结果
 */
class PipelineContext {
public:
    // [修复] 类型更新为 PipelineRunConfig
    PipelineRunConfig config;
    PipelineState state;
    PipelineResult result;

    PipelineContext(const AppConfig& cfg, const fs::path& out_root)
        : config(cfg, out_root) {}
};

#endif // PIPELINE_CONTEXT_HPP
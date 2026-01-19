// core/pipeline/context/pipeline_context.hpp
#ifndef CORE_PIPELINE_CONTEXT_PIPELINE_CONTEXT_HPP_
#define CORE_PIPELINE_CONTEXT_PIPELINE_CONTEXT_HPP_

#include <vector>
#include <filesystem>
#include <map>

#include "common/config/app_config.hpp"
#include "validator/common/ValidatorUtils.hpp"
#include "common/model/daily_log.hpp"

// [重构] 引用 Common 定义的配置结构体
#include "common/config/models/converter_config_models.hpp"

#include <nlohmann/json.hpp> 

namespace fs = std::filesystem;

namespace core::pipeline {

/**
 * @brief Pipeline 任务的运行时配置 (Runtime Config)
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
 */
struct PipelineState {
    std::vector<fs::path> source_files;
    std::vector<fs::path> generated_files;
    
    // 运行时加载的 ConverterConfig (Struct)
    ConverterConfig converter_config;
};

/**
 * @brief Pipeline 的业务数据产出 (Result)
 */
struct PipelineResult {
    // 转换后的核心业务数据
    std::map<std::string, std::vector<DailyLog>> processed_data;
};

/**
 * @brief Pipeline 上下文
 */
class PipelineContext {
public:
    PipelineRunConfig config;
    PipelineState state;
    PipelineResult result;

    // 缓存已验证的 JSON 对象
    std::map<std::string, nlohmann::json> cached_json_outputs;

    PipelineContext(const AppConfig& cfg, const fs::path& out_root)
        : config(cfg, out_root) {}
};

} // namespace core::pipeline

#endif // CORE_PIPELINE_CONTEXT_PIPELINE_CONTEXT_HPP_
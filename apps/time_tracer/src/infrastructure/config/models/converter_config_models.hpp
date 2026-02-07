// infrastructure/config/models/models/converter_config_models.hpp
#ifndef COMMON_CONFIG_MODELS_CONVERTER_CONFIG_MODELS_H_
#define COMMON_CONFIG_MODELS_CONVERTER_CONFIG_MODELS_H_

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// 持续时长映射规则
struct DurationMappingRule {
  int less_than_minutes = 0;
  std::string value;
};

/**
 * @brief 核心转换器配置 (纯数据结构)
 * 完全解耦了 TOML 解析逻辑，仅用于数据传递。
 */
struct ConverterConfig {
  // 基础设置
  std::string remark_prefix;
  std::vector<std::string> header_order;
  std::vector<std::string> wake_keywords;

  // [新增] 自动生成活动的项目路径配置
  // 默认为 "sleep_night"，但可以通过配置文件覆盖
  std::string generated_sleep_project_path = "sleep_night";

  // 映射表
  std::unordered_map<std::string, std::string> top_parent_mapping;
  std::unordered_map<std::string, std::string> text_mapping;
  std::unordered_map<std::string, std::string> text_duration_mapping;

  // 时长规则: Key = event_name (e.g., "rest"), Value = Rules list
  std::unordered_map<std::string, std::vector<DurationMappingRule>>
      duration_mappings;

  // 运行时合并进来的额外配置 (如来自 AppConfig 的 initial_top_parents)
  // 这部分数据通常不在 TOML 中定义，而是在 Pipeline 启动时注入
  std::unordered_map<std::string, std::string> initial_top_parents;
};

#endif  // COMMON_CONFIG_MODELS_CONVERTER_CONFIG_MODELS_H_
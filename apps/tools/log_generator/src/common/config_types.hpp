// common/config_types.hpp
#ifndef COMMON_CONFIG_TYPES_H_
#define COMMON_CONFIG_TYPES_H_

#include <optional>
#include <string>
#include <vector>

// 生成模式枚举
enum class GenerationMode { YearRange, SingleYear };
enum class EventStyle { Point, Interval, Mixed };

// 每日备注配置
struct DailyRemarkConfig {
  std::string prefix;
  std::vector<std::string> contents;
  double generation_chance = 0.5;
  // [新增] 最大行数，默认为 1
  int max_lines = 1;
};

// 活动备注配置
struct ActivityRemarkConfig {
  std::vector<std::string> contents;
  double generation_chance = 0.5;
};

struct ActivityTokenVariant {
  std::string alias_token;
  std::string canonical_token;
};

// 核心运行时配置
struct Config {
  int start_year;
  int end_year;
  int items_per_day;
  GenerationMode mode;
  EventStyle event_style = EventStyle::Point;
  std::optional<int> seed;
  bool enable_nosleep = false;
  bool enable_monthly_average_report = false;
  double nosleep_probability = 1.0;
  std::string output_directory = "dates";
};

// TOML 文件对应的原始数据结构
struct TomlConfigData {
  // 活动来源来自 converter alias mapping bundle。
  // 生成器按映射项持有 alias/canonical 成对 token，并在事件生成时以固定 50%
  // 概率选择 canonical，以便为后续 canonical authoring / ingest 能力准备混合样本。
  std::vector<ActivityTokenVariant> mapped_activities;

  std::optional<DailyRemarkConfig> remarks;
  std::optional<ActivityRemarkConfig> activity_remarks;
  std::vector<std::string> wake_keywords;
  std::optional<double> nosleep_probability;
};

#endif  // COMMON_CONFIG_TYPES_H_

// domain/components/remark_generator.cpp
#include "domain/components/remark_generator.hpp"

#include <format>

RemarkGenerator::RemarkGenerator(const std::optional<DailyRemarkConfig>& config,
                                 std::mt19937& gen)
    : remark_config_(config), gen_(gen) {
  if (remark_config_ && !remark_config_->contents.empty()) {
    selector_.emplace(0, static_cast<int>(remark_config_->contents.size()) - 1);
    should_generate_.emplace(remark_config_->generation_chance);

    // [新增] 初始化行数分布：范围 [1, max_lines]
    lines_count_dist_.emplace(1, remark_config_->max_lines);
  }
}

auto RemarkGenerator::try_generate() -> std::optional<std::string> {
  if (should_generate_ && (*should_generate_)(gen_)) {
    // [新增] 确定本次生成的行数
    int count = (*lines_count_dist_)(gen_);

    std::string result;
    // 简单预估内存，避免多次分配 (假设每行平均30字符)
    result.reserve(count * 18);

    for (int i = 0; i < count; ++i) {
      // 如果不是第一行，先追加换行符
      if (i > 0) {
        result.push_back('\n');
      }
      // 拼接 "prefix" + "随机内容"
      result += remark_config_->prefix;
      result += remark_config_->contents[(*selector_)(gen_)];
    }
    return result;
  }
  return std::nullopt;
}

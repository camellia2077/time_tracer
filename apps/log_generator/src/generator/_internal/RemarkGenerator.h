#pragma once
#include "config/Config.h" // 依赖Config中的数据结构
#include <optional>
#include <string>
#include <random>
#include <memory>

/**
 * @class RemarkGenerator
 * @brief 专门负责生成每日备注。
 */
class RemarkGenerator {
public:
    RemarkGenerator(const std::optional<DailyRemarkConfig>& config, std::mt19937& gen);

    // 尝试生成一条备注
    std::optional<std::string> try_generate();

private:
    const std::optional<DailyRemarkConfig>& remark_config_;
    std::mt19937& gen_; // 引用外部的随机数生成器
    std::unique_ptr<std::uniform_int_distribution<>> selector_;
    std::unique_ptr<std::bernoulli_distribution> should_generate_;
};
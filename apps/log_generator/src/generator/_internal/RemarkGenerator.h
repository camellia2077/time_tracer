// src/generator/_internal/RemarkGenerator.h
#ifndef GENERATOR_INTERNAL_REMARKGENERATOR_H
#define GENERATOR_INTERNAL_REMARKGENERATOR_H

#include "config/Config.h"
#include <optional>
#include <string>
#include <random>

/**
 * @class RemarkGenerator
 * @brief 专门负责生成每日备注。
 */
class RemarkGenerator {
public:
    RemarkGenerator(const std::optional<DailyRemarkConfig>& config, std::mt19937& gen);

    std::optional<std::string> try_generate();

private:
    const std::optional<DailyRemarkConfig>& remark_config_;
    std::mt19937& gen_; 

    std::optional<std::uniform_int_distribution<>> selector_;
    std::optional<std::bernoulli_distribution> should_generate_;
    
    // [新增] 用于生成 1 到 max_lines 之间的随机整数
    std::optional<std::uniform_int_distribution<>> lines_count_dist_;
};

#endif // GENERATOR_INTERNAL_REMARKGENERATOR_H
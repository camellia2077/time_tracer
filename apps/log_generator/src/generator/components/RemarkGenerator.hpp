// generator/components/RemarkGenerator.hpp
#ifndef REMARK_GENERATOR_HPP
#define REMARK_GENERATOR_HPP

#include "config/Config.hpp"
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

#endif // REMARK_GENERATOR_HPP
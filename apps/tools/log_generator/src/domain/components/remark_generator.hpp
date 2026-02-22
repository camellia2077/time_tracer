// domain/components/remark_generator.hpp
#ifndef DOMAIN_COMPONENTS_REMARK_GENERATOR_H_
#define DOMAIN_COMPONENTS_REMARK_GENERATOR_H_

#include <optional>
#include <random>
#include <string>

#include "common/config_types.hpp"

class RemarkGenerator {
 public:
  RemarkGenerator(const std::optional<DailyRemarkConfig>& config,
                  std::mt19937& gen);
  std::optional<std::string> try_generate();

 private:
  const std::optional<DailyRemarkConfig>& remark_config_;
  std::mt19937& gen_;
  std::optional<std::uniform_int_distribution<>> selector_;
  std::optional<std::bernoulli_distribution> should_generate_;
  std::optional<std::uniform_int_distribution<>> lines_count_dist_;
};

#endif  // DOMAIN_COMPONENTS_REMARK_GENERATOR_H_

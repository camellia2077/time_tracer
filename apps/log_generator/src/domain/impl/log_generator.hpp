// domain/impl/log_generator.hpp
#ifndef DOMAIN_IMPL_LOG_GENERATOR_H_
#define DOMAIN_IMPL_LOG_GENERATOR_H_

#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "common/config_types.hpp"
#include "domain/api/i_log_generator.hpp"
#include "domain/components/day_generator.hpp"
#include "domain/strategies/sleep_scheduler.hpp"

class LogGenerator : public ILogGenerator {
 public:
  LogGenerator(
      const Config& config, const std::vector<std::string>& activities,
      const std::optional<DailyRemarkConfig>& remark_config,
      const std::optional<ActivityRemarkConfig>& activity_remark_config,
      const std::vector<std::string>& wake_keywords);

  void generate_for_month(const MonthContext& month_context,
                          std::string& buffer) override;

 private:
  std::mt19937 gen_;
  std::unique_ptr<DayGenerator> day_generator_;
  std::unique_ptr<SleepScheduler> sleep_scheduler_;
};

#endif  // DOMAIN_IMPL_LOG_GENERATOR_H_

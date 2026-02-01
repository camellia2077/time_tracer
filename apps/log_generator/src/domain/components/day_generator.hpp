// domain/components/day_generator.hpp
#ifndef DOMAIN_COMPONENTS_DAY_GENERATOR_H_
#define DOMAIN_COMPONENTS_DAY_GENERATOR_H_

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "domain/components/event_generator.hpp"
#include "domain/components/remark_generator.hpp"

class DayGenerator {
 public:
  DayGenerator(
      int items_per_day, const std::vector<std::string>& activities,
      const std::optional<DailyRemarkConfig>& remark_config,
      const std::optional<ActivityRemarkConfig>& activity_remark_config,
      const std::vector<std::string>& wake_keywords, std::mt19937& gen);

  void generate_for_day(std::string& buffer, int month, int day,
                        bool is_nosleep_day);

 private:
  std::unique_ptr<EventGenerator> event_generator_;
  std::unique_ptr<RemarkGenerator> remark_generator_;
};

#endif  // DOMAIN_COMPONENTS_DAY_GENERATOR_H_

#ifndef GENERATOR_FACADE_LOGGENERATOR_H
#define GENERATOR_FACADE_LOGGENERATOR_H

#include "ILogGenerator.h"
#include "config/Config.h"
#include "generator/_internal/DayGenerator.h"
#include <vector>
#include <string>
#include <optional>
#include <random>
#include <memory>

class LogGenerator : public ILogGenerator {
public:
    LogGenerator(const Config& config,
                 const std::vector<std::string>& activities,
                 const std::optional<DailyRemarkConfig>& remark_config,
                 const std::optional<ActivityRemarkConfig>& activity_remark_config,
                 const std::vector<std::string>& wake_keywords);

    std::string generate_for_month(int year, int month, int days_in_month) override;

private:
    std::mt19937 gen_;
    std::unique_ptr<DayGenerator> day_generator_;

    bool enable_nosleep_;
    int current_sequence_length_ = 1;
    int days_into_sequence_ = 0;
    bool is_in_nosleep_block_ = true;

    std::uniform_int_distribution<> nosleep_length_dist_;
    std::uniform_int_distribution<> normal_length_dist_;
};

#endif // GENERATOR_FACADE_LOGGENERATOR_H
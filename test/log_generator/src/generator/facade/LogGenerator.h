#pragma once
#include "config/Config.h"
#include "generator/_internal/RemarkGenerator.h" // [核心修改] 包含新的头文件
#include "generator/_internal/EventGenerator.h"  // [核心修改] 包含新的头文件
#include <vector>
#include <string>
#include <optional>
#include <random>
#include <memory>
#include <ostream>

/**
 * @class LogGenerator
 * @brief (Facade) 负责协调各个子生成器，生成完整的月度日志文件内容。
 */
class LogGenerator {
public:
    LogGenerator(int items_per_day,
                 const std::vector<std::string>& activities,
                 const std::optional<DailyRemarkConfig>& remark_config);

    // Facade 方法，封装了复杂的生成流程
    std::string generate_for_month(int year, int month, int days_in_month);

private:
    int items_per_day_;
    std::mt19937 gen_;
    std::unique_ptr<RemarkGenerator> remark_generator_;
    std::unique_ptr<EventGenerator> event_generator_;
};
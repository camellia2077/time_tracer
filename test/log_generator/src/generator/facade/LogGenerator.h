#pragma once
#include "config/Config.h"
// [核心修改] 引入新的 DayGenerator
#include "generator/_internal/DayGenerator.h"
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
                 const std::optional<DailyRemarkConfig>& remark_config,
                 const std::optional<ActivityRemarkConfig>& activity_remark_config,
                 const std::vector<std::string>& wake_keywords); // 新增

    // Facade 方法，封装了复杂的生成流程
    std::string generate_for_month(int year, int month, int days_in_month);

private:
    // [核心修改] Facade 现在只持有一个 DayGenerator 实例
    std::mt19937 gen_;
    std::unique_ptr<DayGenerator> day_generator_;
};
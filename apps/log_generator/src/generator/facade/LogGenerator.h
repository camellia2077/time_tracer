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
    LogGenerator(const Config& config, // 修改：传入整个Config对象
                 const std::vector<std::string>& activities,
                 const std::optional<DailyRemarkConfig>& remark_config,
                 const std::optional<ActivityRemarkConfig>& activity_remark_config,
                 const std::vector<std::string>& wake_keywords);

    // Facade 方法，封装了复杂的生成流程
    std::string generate_for_month(int year, int month, int days_in_month);

private:
    std::mt19937 gen_;
    std::unique_ptr<DayGenerator> day_generator_;

    // --- [修改] 通宵模式状态变量 ---
    bool enable_nosleep_;
    int current_sequence_length_ = 1; // 当前区块（通宵或正常）的长度
    int days_into_sequence_ = 0;      // 当前区块已进行的天数
    bool is_in_nosleep_block_ = true; // 当前是否处于通宵区块

    // --- [修改] 更新随机化区块长度的分布 ---
    std::uniform_int_distribution<> nosleep_length_dist_;
    std::uniform_int_distribution<> normal_length_dist_;
};
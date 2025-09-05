#pragma once
#include "Config.h" // 需要用到配置相关的数据结构
#include <vector>
#include <string>
#include <optional>
#include <random>
#include <memory>
#include <ostream>

class LogGenerator {
public:
    LogGenerator(int items_per_day,
                 const std::vector<std::string>& activities,
                 const std::optional<DailyRemarkConfig>& remark_config);

    // [修改] 函数签名新增 year 参数
    std::string generate_for_month(int year, int month, int days_in_month);

private:
    int items_per_day_;
    const std::vector<std::string>& common_activities_;
    const std::optional<DailyRemarkConfig> remark_config_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> dis_minute_;
    std::uniform_int_distribution<> dis_activity_selector_;
    std::unique_ptr<std::uniform_int_distribution<>> dis_remark_selector_;
    std::unique_ptr<std::bernoulli_distribution> dis_remark_should_generate_;
};
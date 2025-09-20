// generator/facade/LogGenerator.cpp
#include "LogGenerator.h"
#include <format>
#include <iterator>

LogGenerator::LogGenerator(const Config& config,
                         const std::vector<std::string>& activities,
                         const std::optional<DailyRemarkConfig>& remark_config,
                         const std::optional<ActivityRemarkConfig>& activity_remark_config,
                         const std::vector<std::string>& wake_keywords)
    : gen_(std::random_device{}()),
      enable_nosleep_(config.enable_nosleep),
      // --- [修改] 更新随机分布范围 ---
      // 通宵区块的长度在1到3天之间
      nosleep_length_dist_(1, 3),
      // 正常区块（恢复期）的长度在2到3天之间
      normal_length_dist_(2, 3)
{
    day_generator_ = std::make_unique<DayGenerator>(config.items_per_day, activities, remark_config, activity_remark_config, wake_keywords, gen_);
    // 首次运行时，随机决定第一个区块是通宵还是正常
    if (enable_nosleep_) {
        is_in_nosleep_block_ = std::bernoulli_distribution(0.5)(gen_);
        if (is_in_nosleep_block_) {
            current_sequence_length_ = nosleep_length_dist_(gen_);
        } else {
            current_sequence_length_ = normal_length_dist_(gen_);
        }
    }
}

// --- [核心修改] 更新 generate_for_month 的逻辑 ---
std::string LogGenerator::generate_for_month(int year, int month, int days_in_month) {
    std::string log_content;
    log_content.reserve((days_in_month + 1) * 280); 

    // 1. 如果是1月份，则首先生成前一年的年份标题和12月31日的内容
    if (month == 1) {
        // 为上一年的数据添加年份标题
        std::format_to(std::back_inserter(log_content), "y{}\n\n", year - 1);
        day_generator_->generate_for_day(log_content, year - 1, 12, 31, false); // 上一年最后一天强制为非通宵
        log_content += '\n';
    }

    // 2. 为当前要生成的月份添加年份标题
    std::format_to(std::back_inserter(log_content), "y{}\n\n", year);

    // 3. 循环委托 DayGenerator 生成当月的所有天
    for (int day = 1; day <= days_in_month; ++day) {
        if (day > 1) {
            log_content += '\n';
        }

        bool is_nosleep_day = false;
        if (enable_nosleep_) {
            // --- [新增规则] 每月最后一天 和 [保留规则] 新年第一天 强制正常 ---
            if ((month == 1 && day == 1) || (day == days_in_month)) {
                is_nosleep_day = false;
                // 重置状态，强制开始一个新的正常区块
                is_in_nosleep_block_ = false;
                days_into_sequence_ = 1; // 已经过了正常区块的第一天
                current_sequence_length_ = normal_length_dist_(gen_);
            } else {
                days_into_sequence_++;
                if (is_in_nosleep_block_) {
                    // 当前在通宵区块
                    is_nosleep_day = true;
                    if (days_into_sequence_ >= current_sequence_length_) {
                        // 通宵区块结束，切换到正常区块
                        is_in_nosleep_block_ = false;
                        days_into_sequence_ = 0;
                        current_sequence_length_ = normal_length_dist_(gen_);
                    }
                } else {
                    // 当前在正常区块
                    is_nosleep_day = false;
                    if (days_into_sequence_ >= current_sequence_length_) {
                        // 正常区块结束，切换到通宵区块
                        is_in_nosleep_block_ = true;
                        days_into_sequence_ = 0;
                        current_sequence_length_ = nosleep_length_dist_(gen_);
                    }
                }
            }
        }
        
        day_generator_->generate_for_day(log_content, year, month, day, is_nosleep_day);
    }
    return log_content;
}
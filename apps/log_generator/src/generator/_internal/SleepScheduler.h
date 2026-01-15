#ifndef GENERATOR_INTERNAL_SLEEPSCHEDULER_H
#define GENERATOR_INTERNAL_SLEEPSCHEDULER_H

#include <random>

class SleepScheduler {
public:
    SleepScheduler(bool enabled, std::mt19937& gen);

    // 在每个月开始时重置状态（对应原代码中 month loop 开始时的逻辑）
    void reset_for_new_month();

    // 决定指定的一天是否为通宵日
    bool determine_if_nosleep(int day, int days_in_month);

private:
    bool enabled_;
    std::mt19937& gen_; // 引用外部生成器或自己持有
    
    // 状态变量移动到这里
    int current_sequence_length_ = 1;
    int days_into_sequence_ = 0;
    bool is_in_nosleep_block_ = false;

    std::uniform_int_distribution<> nosleep_length_dist_;
    std::uniform_int_distribution<> normal_length_dist_;
};

#endif // GENERATOR_INTERNAL_SLEEPSCHEDULER_H
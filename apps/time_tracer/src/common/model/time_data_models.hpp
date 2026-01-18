// common/model/time_data_models.hpp
#ifndef COMMON_MODEL_TIME_DATA_MODELS_HPP_
#define COMMON_MODEL_TIME_DATA_MODELS_HPP_

#include <string>
#include <optional>

// 统一的统计结构 (对应原 GeneratedStats，但字段名为 snake_case)
struct ActivityStats {
    int sleep_night_time = 0;
    int sleep_day_time = 0;
    int sleep_total_time = 0;
    
    int total_exercise_time = 0;
    int cardio_time = 0;
    int anaerobic_time = 0;
    
    int grooming_time = 0;
    int toilet_time = 0;
    int gaming_time = 0;
    
    int recreation_time = 0;
    int recreation_zhihu_time = 0;
    int recreation_bilibili_time = 0;
    int recreation_douyin_time = 0;
    
    int study_time = 0;
};

// 统一的基础活动记录 (对应原 Activity / TimeRecordInternal)
struct BaseActivityRecord {
    long long logical_id = 0;
    long long start_timestamp = 0;
    long long end_timestamp = 0;
    
    std::string start_time_str; // 原 startTime / start
    std::string end_time_str;   // 原 endTime / end
    std::string project_path;
    
    int duration_seconds = 0; // 原 durationSeconds
    std::optional<std::string> remark; // 原 activityRemark
};

#endif // COMMON_MODEL_TIME_DATA_MODELS_HPP_
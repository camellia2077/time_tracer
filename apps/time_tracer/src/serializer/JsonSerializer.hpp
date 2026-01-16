// serializer/JsonSerializer.hpp
#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <vector>
#include <nlohmann/json.hpp>
#include "common/model/DailyLog.hpp"

namespace serializer {

/**
 * @brief 序列化模块的外观类 (Facade)
 * 负责调度内部核心组件 (LogSerializer, LogDeserializer) 完成转换任务。
 */
class JsonSerializer {
public:
    // Struct -> JSON
    static nlohmann::json serializeDay(const DailyLog& day);
    static nlohmann::json serializeDays(const std::vector<DailyLog>& days);

    // JSON -> Struct
    static DailyLog deserializeDay(const nlohmann::json& day_json);
    static std::vector<DailyLog> deserializeDays(const nlohmann::json& json_array);
};

} // namespace serializer

#endif // JSON_SERIALIZER_HPP

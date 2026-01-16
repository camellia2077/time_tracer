// serializer/core/LogDeserializer.hpp
#ifndef LOG_DESERIALIZER_HPP
#define LOG_DESERIALIZER_HPP

#include <nlohmann/json.hpp>
#include "common/model/DailyLog.hpp"

namespace serializer::core {

class LogDeserializer {
public:
    /**
     * @brief 将 JSON 对象反序列化为 DailyLog 结构体
     * @throws nlohmann::json::exception 解析错误时抛出
     */
    static DailyLog deserialize(const nlohmann::json& day_json);
};

} // namespace serializer::core

#endif // LOG_DESERIALIZER_HPP

// serializer/core/LogDeserializer.hpp
#ifndef SERIALIZER_CORE_LOG_DESERIALIZER_HPP_
#define SERIALIZER_CORE_LOG_DESERIALIZER_HPP_

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

#endif // SERIALIZER_CORE_LOG_DESERIALIZER_HPP_

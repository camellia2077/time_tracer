// serializer/core/LogSerializer.hpp
#ifndef LOG_SERIALIZER_HPP
#define LOG_SERIALIZER_HPP

#include <nlohmann/json.hpp>
#include "common/model/DailyLog.hpp"

namespace serializer::core {

class LogSerializer {
public:
    /**
     * @brief 将单个 DailyLog 结构体序列化为 JSON 对象
     */
    static nlohmann::json serialize(const DailyLog& day);
};

} // namespace serializer::core

#endif // LOG_SERIALIZER_HPP

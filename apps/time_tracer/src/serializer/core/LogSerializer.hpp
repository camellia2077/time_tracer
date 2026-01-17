// serializer/core/LogSerializer.hpp
#ifndef SERIALIZER_CORE_LOG_SERIALIZER_HPP_
#define SERIALIZER_CORE_LOG_SERIALIZER_HPP_

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

#endif // SERIALIZER_CORE_LOG_SERIALIZER_HPP_

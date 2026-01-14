// converter/validator/common/ValidatorUtils.hpp
#ifndef VALIDATOR_UTILS_HPP
#define VALIDATOR_UTILS_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

// [架构优化] 引入位于 common 的枚举定义，解耦定义权
#include "common/types/DateCheckMode.hpp"

// 共享的错误类型枚举
enum class ErrorType {
    FileAccess,
    Structural,
    LineFormat,
    TimeDiscontinuity,
    MissingSleepNight,
    Logical,
    DateContinuity,
    IncorrectDayCountForMonth,
    Source_RemarkAfterEvent,
    Source_NoDateAtStart,
    UnrecognizedActivity,
    Source_InvalidLineFormat,
    Source_MissingYearHeader, 
    Json_TooFewActivities 
};

// 共享的错误结构体
struct Error {
    int line_number;
    std::string message;
    ErrorType type;

    bool operator<(const Error& other) const {
        if (line_number != other.line_number) return line_number < other.line_number;
        if (type != other.type) return type < other.type;
        return message < other.message;
    }
};

void printGroupedErrors(const std::string& filename, const std::set<Error>& errors);

#endif // VALIDATOR_UTILS_HPP
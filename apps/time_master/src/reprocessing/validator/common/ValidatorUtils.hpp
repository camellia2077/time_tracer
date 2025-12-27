// reprocessing/validator/common/ValidatorUtils.hpp
#ifndef VALIDATOR_UTILS_HPP
#define VALIDATOR_UTILS_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

// --- [核心修改] 新增日期检查模式枚举 ---
enum class DateCheckMode {
    None,       // 不检查
    Continuity, // 只检查连续性 (1号到当前存在的最大日期)
    Full        // 检查完整性 (1号到月底)
};

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
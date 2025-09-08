// time_master/reprocessing/validator/ValidatorUtils.hpp
#ifndef VALIDATOR_UTILS_HPP
#define VALIDATOR_UTILS_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

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
    Source_MissingYearHeader, // 源文件缺少年份标题,例如y2025
    Json_TooFewActivities // 确保一天至少有两个活动
};

// 共享的错误结构体
struct Error {
    int line_number;
    std::string message;
    ErrorType type;

    // 重载小于运算符，以便放入 std::set 中
    bool operator<(const Error& other) const {
        if (line_number != other.line_number) return line_number < other.line_number;
        if (type != other.type) return type < other.type;
        return message < other.message;
    }
};

// --- 共享函数 ---

// 打印分组的错误信息
void printGroupedErrors(const std::string& filename, const std::set<Error>& errors, const std::string& error_log_path);

// 去除字符串两端的空白字符
std::string trim(const std::string& str);

#endif // VALIDATOR_UTILS_HPP
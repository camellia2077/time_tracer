// validator/common/ValidatorUtils.hpp
#ifndef VALIDATOR_COMMON_VALIDATOR_UTILS_HPP_
#define VALIDATOR_COMMON_VALIDATOR_UTILS_HPP_

#include <string>
#include <vector>
#include <set>
#include <map>

#include "common/types/date_check_mode.hpp"

namespace validator {

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

} // namespace validator

#endif // VALIDATOR_COMMON_VALIDATOR_UTILS_HPP_
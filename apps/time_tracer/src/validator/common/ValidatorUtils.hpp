#ifndef VALIDATOR_UTILS_HPP
#define VALIDATOR_UTILS_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

#include "common/types/DateCheckMode.hpp"

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

#endif // VALIDATOR_UTILS_HPP



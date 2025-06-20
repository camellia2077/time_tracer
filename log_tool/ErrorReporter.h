// ErrorReporter.h

#ifndef ERROR_REPORTER_H
#define ERROR_REPORTER_H

#include <string>
#include <set>
#include "FormatValidator.h" // Needed for the Error and ErrorType definitions

namespace ErrorReporter {

/**
 * @brief Groups errors by type and prints them to the console and a log file.
 * * @param filename The name of the file that was validated.
 * @param errors A set containing the errors found during validation.
 * @param error_log_path The path to the file where errors should be logged.
 */
void printGroupedErrors(const std::string& filename, const std::set<FormatValidator::Error>& errors, const std::string& error_log_path);

} // namespace ErrorReporter

#endif // ERROR_REPORTER_H
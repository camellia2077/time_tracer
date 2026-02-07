// shared/types/exit_codes.hpp
#ifndef SHARED_TYPES_EXIT_CODES_H_
#define SHARED_TYPES_EXIT_CODES_H_

#include <cstdint>

/**
 * @brief Application exit codes for CLI.
 * Used to provide granular feedback to automated tests and shell users.
 */
enum class AppExitCode : int32_t {
  kSuccess = 0,
  kGenericError = 1,
  kCommandNotFound = 2,
  kInvalidArguments = 3,
  kDatabaseError = 4,
  kIoError = 5,
  kLogicError = 6,
  kConfigError = 7,
  kMemoryError = 8,
  kUnknownError = 9
};

#endif  // SHARED_TYPES_EXIT_CODES_H_

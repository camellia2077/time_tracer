// shared/types/exceptions.hpp
#ifndef SHARED_TYPES_EXCEPTIONS_H_
#define SHARED_TYPES_EXCEPTIONS_H_

#include <stdexcept>
#include <string>

namespace time_tracer::common {

/**
 * @brief Base class for application-specific exceptions.
 */
class AppError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

/**
 * @brief Exception thrown for database-related errors.
 */
class DatabaseError : public AppError {
 public:
  using AppError::AppError;
};

/**
 * @brief Exception thrown for configuration-related errors.
 */
class ConfigError : public AppError {
 public:
  using AppError::AppError;
};

/**
 * @brief Exception thrown for I/O-related errors.
 */
class IoError : public AppError {
 public:
  using AppError::AppError;
};

/**
 * @brief Exception thrown for logic-related errors.
 */
class LogicError : public AppError {
 public:
  using AppError::AppError;
};

/**
 * @brief Exception thrown for plugin ABI mismatch/incompatibility.
 */
class DllCompatibilityError : public LogicError {
 public:
  using LogicError::LogicError;
};

}  // namespace time_tracer::common

#endif  // SHARED_TYPES_EXCEPTIONS_H_

// common/exit_code.hpp
#ifndef COMMON_EXIT_CODE_H_
#define COMMON_EXIT_CODE_H_

namespace App {

enum class ExitCode : int {
  kSuccess = 0,
  kCliError = 2,
  kMissingRequestConfig = 3,
  kRuntimeConfigLoadFailed = 4,
  kGenerationFailed = 5,
  kInternalError = 10,
};

[[nodiscard]] constexpr auto to_status_code(ExitCode code) -> int {
  return static_cast<int>(code);
}

}  // namespace App

#endif  // COMMON_EXIT_CODE_H_

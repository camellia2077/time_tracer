#ifndef SHARED_TYPES_INSIGHTS_ERRORS_H_
#define SHARED_TYPES_INSIGHTS_ERRORS_H_

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain/errors/error_codes.hpp"
#include "shared/types/exceptions.hpp"

namespace tracer_core::common {

class InsightsContractError : public LogicError {
 public:
  InsightsContractError(std::string message, std::string error_code,
                         std::string error_category,
                         std::vector<std::string> hints = {})
      : LogicError(message),
        error_code_(std::move(error_code)),
        error_category_(std::move(error_category)),
        hints_(std::move(hints)) {}

  [[nodiscard]] auto error_code() const -> const std::string& {
    return error_code_;
  }

  [[nodiscard]] auto error_category() const -> const std::string& {
    return error_category_;
  }

  [[nodiscard]] auto hints() const -> const std::vector<std::string>& {
    return hints_;
  }

 private:
  std::string error_code_;
  std::string error_category_;
  std::vector<std::string> hints_;
};

class InsightsTargetNotFoundError final : public InsightsContractError {
 public:
  InsightsTargetNotFoundError(std::string_view target_type,
                            std::string_view target_value)
      : InsightsContractError(
            BuildMessage(target_type, target_value),
            std::string(
                tracer_core::domain::errors::codes::insights::kTargetNotFound),
            "insights",
            {"Check that the requested insights target exists in the current "
             "database."}) {}

 private:
  [[nodiscard]] static auto BuildMessage(std::string_view target_type,
                                         std::string_view target_value)
      -> std::string {
    return "Insights target not found: " + std::string(target_type) + " `" +
           std::string(target_value) + "`.";
  }
};

template <typename TOutput>
auto ApplyInsightsContract(TOutput& output,
                            const InsightsContractError& error) -> void {
  output.error_contract.error_code = error.error_code();
  output.error_contract.error_category = error.error_category();
  output.error_contract.hints = error.hints();
}

}  // namespace tracer_core::common

#endif  // SHARED_TYPES_INSIGHTS_ERRORS_H_

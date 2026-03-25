// infra/io/processed_json_validation.hpp
#ifndef INFRASTRUCTURE_IO_PROCESSED_JSON_VALIDATION_H_
#define INFRASTRUCTURE_IO_PROCESSED_JSON_VALIDATION_H_

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "application/ports/pipeline/i_processed_data_loader.hpp"

namespace infrastructure::io {

struct ProcessedJsonDayValidationInput {
  std::string date;
  size_t activity_count = 0;
  bool has_day_object = false;
  bool has_headers_object = false;
  bool has_date_string = false;
  bool has_activities_array = false;
};

struct ProcessedJsonValidationInput {
  bool has_root_array = false;
  std::vector<ProcessedJsonDayValidationInput> days;
};

[[nodiscard]] auto BuildProcessedJsonValidationInput(
    const nlohmann::json& payload) -> ProcessedJsonValidationInput;

[[nodiscard]] auto CollectProcessedJsonValidationErrors(
    std::string_view source, const ProcessedJsonValidationInput& input)
    -> std::vector<tracer_core::application::ports::ProcessedDataLoadError>;

}  // namespace infrastructure::io

#endif  // INFRASTRUCTURE_IO_PROCESSED_JSON_VALIDATION_H_

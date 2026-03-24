#ifndef APPLICATION_DTO_PIPELINE_REQUESTS_HPP_
#define APPLICATION_DTO_PIPELINE_REQUESTS_HPP_

#include <string>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

namespace tracer_core::core::dto {

struct ConvertRequest {
  std::string input_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool save_processed_output = false;
  bool validate_logic = true;
  bool validate_structure = true;
};

struct IngestRequest {
  std::string input_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool save_processed_output = false;
  IngestMode ingest_mode = IngestMode::kStandard;
};

struct ImportRequest {
  std::string processed_path;
};

struct ValidateStructureRequest {
  std::string input_path;
};

struct ValidateLogicRequest {
  std::string input_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_PIPELINE_REQUESTS_HPP_

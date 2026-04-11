#ifndef APPLICATION_DTO_PIPELINE_REQUESTS_HPP_
#define APPLICATION_DTO_PIPELINE_REQUESTS_HPP_

#include <string>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"
#include "domain/types/time_order_mode.hpp"

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

struct RecordActivityAtomicallyRequest {
  std::string target_date_iso;
  std::string raw_activity_name;
  std::string remark;
  std::string preferred_txt_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  TimeOrderMode time_order_mode = TimeOrderMode::kStrictCalendar;
};

struct DefaultTxtDayMarkerRequest {
  std::string selected_month;
  std::string target_date_iso;
};

struct ResolveTxtDayBlockRequest {
  std::string content;
  std::string day_marker;
  std::string selected_month;
};

struct ReplaceTxtDayBlockRequest {
  std::string content;
  std::string day_marker;
  std::string edited_day_body;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_PIPELINE_REQUESTS_HPP_

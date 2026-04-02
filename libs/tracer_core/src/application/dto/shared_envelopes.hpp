#ifndef APPLICATION_DTO_SHARED_ENVELOPES_HPP_
#define APPLICATION_DTO_SHARED_ENVELOPES_HPP_

#include <optional>
#include <string>
#include <vector>

namespace tracer_core::core::dto {

// Shared aggregate envelopes intentionally remain outside capability-owned DTO
// families because multiple application APIs and shell/runtime bridges reuse
// them directly.
struct ErrorContractFields {
  std::string error_code;
  std::string error_category;
  std::vector<std::string> hints;
};

struct ReportWindowMetadata {
  bool has_records = false;
  int matched_day_count = 0;
  int matched_record_count = 0;
  std::string start_date;
  std::string end_date;
  int requested_days = 0;
};

struct OperationAck {
  bool ok = true;
  std::string error_message;
  ErrorContractFields error_contract;
};

struct TextOutput {
  bool ok = true;
  std::string content;
  std::string error_message;
  ErrorContractFields error_contract;
  std::optional<ReportWindowMetadata> report_window_metadata;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_SHARED_ENVELOPES_HPP_

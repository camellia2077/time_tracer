#ifndef APPLICATION_DTO_PIPELINE_RESPONSES_HPP_
#define APPLICATION_DTO_PIPELINE_RESPONSES_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tracer_core::core::dto {

struct IngestSyncStatusRequest {
  std::vector<std::string> months;
};

struct IngestSyncStatusEntry {
  std::string month_key;
  std::string txt_relative_path;
  std::string txt_content_hash_sha256;
  std::int64_t ingested_at_unix_ms = 0;
};

struct IngestSyncStatusOutput {
  bool ok = true;
  std::vector<IngestSyncStatusEntry> items;
  std::string error_message;
};

struct RecordActivityAtomicallyResponse {
  bool ok = false;
  std::string message;
  std::string operation_id;
  std::vector<std::string> warnings;
  bool rollback_failed = false;
  std::optional<std::string> retained_transaction_root;
};

struct DefaultTxtDayMarkerResponse {
  bool ok = false;
  std::string normalized_day_marker;
  std::string error_message;
};

struct ResolveTxtDayBlockResponse {
  bool ok = false;
  std::string normalized_day_marker;
  bool found = false;
  bool is_marker_valid = false;
  bool can_save = false;
  std::string day_body;
  std::optional<std::string> day_content_iso_date;
  std::string error_message;
};

struct ReplaceTxtDayBlockResponse {
  bool ok = false;
  std::string normalized_day_marker;
  bool found = false;
  bool is_marker_valid = false;
  std::string updated_content;
  std::string error_message;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_PIPELINE_RESPONSES_HPP_

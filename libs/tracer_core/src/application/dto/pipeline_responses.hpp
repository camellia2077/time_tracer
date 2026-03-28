#ifndef APPLICATION_DTO_PIPELINE_RESPONSES_HPP_
#define APPLICATION_DTO_PIPELINE_RESPONSES_HPP_

#include <cstdint>
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

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_PIPELINE_RESPONSES_HPP_

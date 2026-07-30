#ifndef APPLICATION_DTO_EXCHANGE_CONTENT_HPP_
#define APPLICATION_DTO_EXCHANGE_CONTENT_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "domain/types/date_check_mode.hpp"

namespace tracer_core::core::dto {

enum class TracerExchangeContentEntryKind {
  kManifest,
  kConfig,
  kPayload,
};

struct TracerExchangeTextPayloadItem {
  std::string relative_path_hint;
  std::string content;
};

struct TracerExchangeContentEntry {
  std::string relative_path;
  std::vector<std::uint8_t> data;
  TracerExchangeContentEntryKind kind =
      TracerExchangeContentEntryKind::kConfig;
  bool required = true;
  bool text = true;
};

struct TracerExchangeContentManifest {
  std::string package_type = "tracer_exchange";
  std::int64_t package_version = 6;
  std::string producer_platform;
  std::string producer_app;
  std::string created_at_utc;
  std::string source_root_name;
  std::string config_root = "config/user";
  std::vector<std::string> config_files;
  std::string payload_root = "payload";
  std::vector<std::string> payload_files;
};

struct TracerExchangeExportContent {
  TracerExchangeContentManifest manifest;
  std::vector<TracerExchangeContentEntry> entries;
};

struct TracerExchangeEncodedContent {
  std::vector<std::uint8_t> package_bytes;
};

struct TracerExchangeContentEncodingResult {
  bool ok = true;
  TracerExchangeEncodedContent content;
  std::string error_message;
};

struct TracerExchangeContentRequest {
  std::filesystem::path config_user_root_path;
  std::filesystem::path active_converter_main_config_path;
  std::filesystem::path input_text_root_path;
  std::vector<TracerExchangeTextPayloadItem> input_text_payloads;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  std::string logical_source_root_name;
  std::string producer_platform;
  std::string producer_app;
};

struct TracerExchangeContentResult {
  bool ok = true;
  TracerExchangeExportContent content;
  std::string error_message;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_EXCHANGE_CONTENT_HPP_

#ifndef APPLICATION_DTO_EXCHANGE_REQUESTS_HPP_
#define APPLICATION_DTO_EXCHANGE_REQUESTS_HPP_

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

#include "domain/types/date_check_mode.hpp"

namespace tracer_core::core::dto {

enum class TracerExchangeSecurityLevel {
  kMin,
  kInteractive,
  kModerate,
  kHigh,
  kMax,
};

enum class TracerExchangeProgressControl {
  kContinue,
  kCancel,
};

struct TracerExchangeProgressSnapshot {
  std::filesystem::path input_root_path;
  std::filesystem::path output_root_path;
  std::filesystem::path current_input_path;
  std::filesystem::path current_output_path;
  std::string current_item;
  std::string current_group_label;
  std::size_t phase_index = 0;
  std::size_t phase_count = 0;
  std::size_t done_count = 0;
  std::size_t total_count = 0;
  std::size_t group_index = 0;
  std::size_t group_count = 0;
  std::size_t file_index_in_group = 0;
  std::size_t file_count_in_group = 0;
  std::size_t current_file_index = 0;
  std::size_t total_files = 0;
  std::uint64_t current_file_done_bytes = 0;
  std::uint64_t current_file_total_bytes = 0;
  std::uint64_t overall_done_bytes = 0;
  std::uint64_t overall_total_bytes = 0;
  std::uint64_t speed_bytes_per_sec = 0;
  std::uint64_t remaining_bytes = 0;
  std::uint64_t eta_seconds = 0;
  bool is_encrypt_operation = true;
  std::string phase;
};

using TracerExchangeProgressObserver = std::function<
    TracerExchangeProgressControl(const TracerExchangeProgressSnapshot&)>;

struct TracerExchangeExportRequest {
  std::filesystem::path input_text_root_path;
  std::filesystem::path requested_output_path;
  std::filesystem::path active_converter_main_config_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  std::string passphrase;
  std::string producer_platform;
  std::string producer_app;
  TracerExchangeSecurityLevel security_level =
      TracerExchangeSecurityLevel::kInteractive;
  TracerExchangeProgressObserver progress_observer{};
};

struct TracerExchangeImportRequest {
  std::filesystem::path input_tracer_path;
  std::filesystem::path active_text_root_path;
  std::filesystem::path active_converter_main_config_path;
  std::filesystem::path runtime_work_root;
  DateCheckMode date_check_mode = DateCheckMode::kContinuity;
  std::string passphrase;
  TracerExchangeProgressObserver progress_observer{};
};

struct TracerExchangeInspectRequest {
  std::filesystem::path input_tracer_path;
  std::string passphrase;
  TracerExchangeProgressObserver progress_observer{};
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_EXCHANGE_REQUESTS_HPP_

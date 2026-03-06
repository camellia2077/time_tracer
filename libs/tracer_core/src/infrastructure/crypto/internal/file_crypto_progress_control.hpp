// infrastructure/crypto/internal/file_crypto_progress_control.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_PROGRESS_CONTROL_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_PROGRESS_CONTROL_HPP_

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>

#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::infrastructure::crypto::internal {

namespace fs = std::filesystem;

struct ProgressFileDescriptor {
  fs::path input_path;
  fs::path output_path;
  std::string group_label;
  std::size_t group_index = 0;
  std::size_t group_file_index = 0;
  std::size_t group_file_count = 0;
  std::size_t file_index = 0;
  std::size_t total_files = 0;
  std::uint64_t input_size_bytes = 0;
};

class ProgressReporter {
 public:
  ProgressReporter(FileCryptoOperation operation,
                   const FileCryptoOptions* options);

  auto BeginScan(const fs::path& input_root, const fs::path& output_root)
      -> FileCryptoResult;

  auto SetAggregateTotals(std::size_t total_files, std::uint64_t total_bytes,
                          std::size_t group_count, bool force_emit)
      -> FileCryptoResult;

  auto SetCurrentFile(const ProgressFileDescriptor& descriptor,
                      std::uint64_t base_overall_done_bytes)
      -> FileCryptoResult;

  auto SetPhase(FileCryptoPhase phase, bool force_emit) -> FileCryptoResult;
  auto UpdateCurrentFileProgress(std::uint64_t done_bytes) -> FileCryptoResult;
  auto FinishCurrentFile() -> FileCryptoResult;
  auto MarkCompleted() -> FileCryptoResult;
  auto MarkCancelled() -> FileCryptoResult;
  auto MarkFailed() -> FileCryptoResult;

  [[nodiscard]] auto CurrentOverallDoneBytes() const -> std::uint64_t;

 private:
  [[nodiscard]] auto IsTokenCancelled() const -> bool;
  [[nodiscard]] auto ShouldEmit(bool force_emit) const -> bool;
  auto UpdateRuntimeEstimates(std::chrono::steady_clock::time_point emit_time)
      -> void;
  auto Emit(bool force_emit) -> FileCryptoResult;

  const FileCryptoOptions* options_ = nullptr;
  FileCryptoProgressSnapshot snapshot_{};
  std::uint64_t current_file_base_overall_done_bytes_ = 0;
  std::chrono::steady_clock::time_point reporter_start_time_{};
  std::chrono::steady_clock::time_point last_emit_time_{};
  std::uint64_t last_emit_overall_done_bytes_ = 0;
  bool has_last_emit_ = false;
};

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_PROGRESS_CONTROL_HPP_

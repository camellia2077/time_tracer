// infrastructure/crypto/internal/file_crypto_progress_control.cpp
#include "infrastructure/crypto/internal/file_crypto_progress_control.hpp"

#include <algorithm>

#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {

ProgressReporter::ProgressReporter(FileCryptoOperation operation,
                                   const FileCryptoOptions* options)
    : options_(options),
      reporter_start_time_(std::chrono::steady_clock::now()) {
  snapshot_.operation = operation;
}

auto ProgressReporter::BeginScan(const fs::path& input_root,
                                 const fs::path& output_root)
    -> FileCryptoResult {
  snapshot_.input_root_path = input_root;
  snapshot_.output_root_path = output_root;
  snapshot_.phase = FileCryptoPhase::kScan;
  return Emit(true);
}

auto ProgressReporter::SetAggregateTotals(std::size_t total_files,
                                          std::uint64_t total_bytes,
                                          std::size_t group_count,
                                          bool force_emit) -> FileCryptoResult {
  snapshot_.total_files = total_files;
  snapshot_.overall_total_bytes = total_bytes;
  snapshot_.group_count = group_count;
  return Emit(force_emit);
}

auto ProgressReporter::SetCurrentFile(const ProgressFileDescriptor& descriptor,
                                      std::uint64_t base_overall_done_bytes)
    -> FileCryptoResult {
  current_file_base_overall_done_bytes_ = base_overall_done_bytes;
  snapshot_.current_input_path = descriptor.input_path;
  snapshot_.current_output_path = descriptor.output_path;
  snapshot_.current_group_label = descriptor.group_label;
  snapshot_.group_index = descriptor.group_index;
  snapshot_.file_index_in_group = descriptor.group_file_index;
  snapshot_.file_count_in_group = descriptor.group_file_count;
  snapshot_.current_file_index = descriptor.file_index;
  snapshot_.current_file_total_bytes = descriptor.input_size_bytes;
  snapshot_.current_file_done_bytes = 0;
  snapshot_.overall_done_bytes = base_overall_done_bytes;
  return Emit(true);
}

auto ProgressReporter::SetPhase(FileCryptoPhase phase, bool force_emit)
    -> FileCryptoResult {
  snapshot_.phase = phase;
  return Emit(force_emit);
}

auto ProgressReporter::UpdateCurrentFileProgress(std::uint64_t done_bytes)
    -> FileCryptoResult {
  const std::uint64_t capped_done =
      std::min(done_bytes, snapshot_.current_file_total_bytes);
  snapshot_.current_file_done_bytes = capped_done;
  snapshot_.overall_done_bytes =
      current_file_base_overall_done_bytes_ + snapshot_.current_file_done_bytes;
  return Emit(false);
}

auto ProgressReporter::FinishCurrentFile() -> FileCryptoResult {
  snapshot_.current_file_done_bytes = snapshot_.current_file_total_bytes;
  snapshot_.overall_done_bytes = current_file_base_overall_done_bytes_ +
                                 snapshot_.current_file_total_bytes;
  return Emit(true);
}

auto ProgressReporter::MarkCompleted() -> FileCryptoResult {
  snapshot_.phase = FileCryptoPhase::kCompleted;
  snapshot_.overall_done_bytes = snapshot_.overall_total_bytes;
  return Emit(true);
}

auto ProgressReporter::MarkCancelled() -> FileCryptoResult {
  snapshot_.phase = FileCryptoPhase::kCancelled;
  return Emit(true);
}

auto ProgressReporter::MarkFailed() -> FileCryptoResult {
  snapshot_.phase = FileCryptoPhase::kFailed;
  return Emit(true);
}

auto ProgressReporter::CurrentOverallDoneBytes() const -> std::uint64_t {
  return snapshot_.overall_done_bytes;
}

auto ProgressReporter::IsTokenCancelled() const -> bool {
  return options_ != nullptr && options_->cancel_token != nullptr &&
         options_->cancel_token->IsCancelled();
}

auto ProgressReporter::ShouldEmit(bool force_emit) const -> bool {
  if (force_emit || !has_last_emit_) {
    return true;
  }
  if (options_ == nullptr) {
    return false;
  }

  const auto now = std::chrono::steady_clock::now();
  const auto min_interval = options_->progress_min_interval;
  const bool interval_ready =
      min_interval.count() <= 0 || now - last_emit_time_ >= min_interval;

  const std::uint64_t bytes_delta =
      snapshot_.overall_done_bytes >= last_emit_overall_done_bytes_
          ? snapshot_.overall_done_bytes - last_emit_overall_done_bytes_
          : 0;
  const std::uint64_t min_bytes_delta = options_->progress_min_bytes_delta;
  const bool bytes_ready =
      min_bytes_delta == 0 || bytes_delta >= min_bytes_delta;

  return interval_ready || bytes_ready;
}

auto ProgressReporter::UpdateRuntimeEstimates(
    std::chrono::steady_clock::time_point emit_time) -> void {
  const std::uint64_t capped_done =
      std::min(snapshot_.overall_done_bytes, snapshot_.overall_total_bytes);
  snapshot_.remaining_bytes = snapshot_.overall_total_bytes - capped_done;

  if (snapshot_.phase == FileCryptoPhase::kCompleted ||
      snapshot_.phase == FileCryptoPhase::kCancelled ||
      snapshot_.phase == FileCryptoPhase::kFailed) {
    snapshot_.speed_bytes_per_sec = 0;
    snapshot_.eta_seconds = 0;
    return;
  }

  std::uint64_t speed_bytes_per_sec = 0;
  if (has_last_emit_) {
    const auto kElapsed = emit_time - last_emit_time_;
    if (kElapsed > std::chrono::steady_clock::duration::zero()) {
      const auto kDeltaSeconds =
          std::chrono::duration_cast<std::chrono::duration<long double>>(
              kElapsed)
              .count();
      const std::uint64_t kBytesDelta =
          snapshot_.overall_done_bytes >= last_emit_overall_done_bytes_
              ? snapshot_.overall_done_bytes - last_emit_overall_done_bytes_
              : 0;
      if (kDeltaSeconds > 0.0L && kBytesDelta > 0) {
        const long double kSpeed =
            static_cast<long double>(kBytesDelta) / kDeltaSeconds;
        if (kSpeed > 0.0L) {
          speed_bytes_per_sec = static_cast<std::uint64_t>(kSpeed);
        }
      }
    }
  }
  if (speed_bytes_per_sec == 0 && snapshot_.overall_done_bytes > 0) {
    const auto kElapsedSinceStart = emit_time - reporter_start_time_;
    if (kElapsedSinceStart > std::chrono::steady_clock::duration::zero()) {
      const auto kElapsedSeconds =
          std::chrono::duration_cast<std::chrono::duration<long double>>(
              kElapsedSinceStart)
              .count();
      if (kElapsedSeconds > 0.0L) {
        const long double kSpeed =
            static_cast<long double>(snapshot_.overall_done_bytes) /
            kElapsedSeconds;
        if (kSpeed > 0.0L) {
          speed_bytes_per_sec = static_cast<std::uint64_t>(kSpeed);
        }
      }
    }
  }

  snapshot_.speed_bytes_per_sec = speed_bytes_per_sec;
  snapshot_.eta_seconds =
      speed_bytes_per_sec > 0 && snapshot_.remaining_bytes > 0
          ? (snapshot_.remaining_bytes + speed_bytes_per_sec - 1) /
                speed_bytes_per_sec
          : 0;
}

auto ProgressReporter::Emit(bool force_emit) -> FileCryptoResult {
  if (IsTokenCancelled() && snapshot_.phase != FileCryptoPhase::kCompleted &&
      snapshot_.phase != FileCryptoPhase::kFailed) {
    snapshot_.phase = FileCryptoPhase::kCancelled;
    return MakeError(FileCryptoError::kCancelled, "Operation cancelled.");
  }

  if (options_ == nullptr || !options_->progress_callback) {
    return {};
  }
  if (!ShouldEmit(force_emit)) {
    return {};
  }

  const auto kEmitTime = std::chrono::steady_clock::now();
  UpdateRuntimeEstimates(kEmitTime);
  const FileCryptoControl kControl = options_->progress_callback(snapshot_);
  last_emit_time_ = kEmitTime;
  last_emit_overall_done_bytes_ = snapshot_.overall_done_bytes;
  has_last_emit_ = true;

  if (kControl == FileCryptoControl::kCancel &&
      snapshot_.phase != FileCryptoPhase::kCompleted &&
      snapshot_.phase != FileCryptoPhase::kFailed) {
    snapshot_.phase = FileCryptoPhase::kCancelled;
    return MakeError(FileCryptoError::kCancelled, "Operation cancelled.");
  }
  if (IsTokenCancelled() && snapshot_.phase != FileCryptoPhase::kCompleted &&
      snapshot_.phase != FileCryptoPhase::kFailed) {
    snapshot_.phase = FileCryptoPhase::kCancelled;
    return MakeError(FileCryptoError::kCancelled, "Operation cancelled.");
  }
  return {};
}

}  // namespace tracer_core::infrastructure::crypto::internal

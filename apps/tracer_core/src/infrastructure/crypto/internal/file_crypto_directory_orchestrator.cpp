// infrastructure/crypto/internal/file_crypto_directory_orchestrator.cpp
#include "infrastructure/crypto/internal/file_crypto_directory_orchestrator.hpp"

#include <optional>

#include "infrastructure/crypto/internal/file_crypto_backend_engine.hpp"
#include "infrastructure/crypto/internal/file_crypto_common.hpp"
#include "infrastructure/crypto/internal/file_crypto_directory_plan.hpp"
#include "infrastructure/crypto/internal/file_crypto_progress_control.hpp"

namespace tracer_core::infrastructure::crypto::internal {
namespace {

auto ToProgressDescriptor(const DirectoryTaskPlanEntry& entry)
    -> ProgressFileDescriptor {
  ProgressFileDescriptor descriptor{};
  descriptor.input_path = entry.input_path;
  descriptor.output_path = entry.output_path;
  descriptor.group_label = entry.group_label;
  descriptor.group_index = entry.group_index;
  descriptor.group_file_index = entry.group_file_index;
  descriptor.group_file_count = entry.group_file_count;
  descriptor.file_index = entry.file_index;
  descriptor.total_files = entry.total_files;
  descriptor.input_size_bytes = entry.input_size_bytes;
  return descriptor;
}

}  // namespace

auto RunDirectoryCrypto(FileCryptoOperation operation,
                        const fs::path& input_root_path,
                        const fs::path& output_root_path,
                        std::string_view passphrase,
                        std::string_view input_extension_lower,
                        std::string_view output_extension_lower,
                        const FileCryptoOptions& options)
    -> FileCryptoBatchResult {
  FileCryptoBatchResult batch{};

  if (passphrase.empty()) {
    batch.status = MakeError(FileCryptoError::kInvalidArgument,
                             "Passphrase must not be empty.");
    return batch;
  }

  ProgressReporter reporter(operation, &options);
  if (const auto scan_begin =
          reporter.BeginScan(input_root_path, output_root_path);
      !scan_begin.ok()) {
    batch.status = scan_begin;
    batch.cancelled = IsCancelledError(scan_begin);
    return batch;
  }

  auto [plan_result, plan] =
      BuildDirectoryTaskPlan(input_root_path, output_root_path,
                             input_extension_lower, output_extension_lower);
  if (!plan_result.ok()) {
    batch.status = plan_result;
    batch.cancelled = IsCancelledError(plan_result);
    if (batch.cancelled) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return batch;
  }

  if (const auto totals_result = reporter.SetAggregateTotals(
          plan.entries.size(), plan.total_input_bytes, plan.group_count, true);
      !totals_result.ok()) {
    batch.status = totals_result;
    batch.cancelled = IsCancelledError(totals_result);
    if (batch.cancelled) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return batch;
  }

  batch.total_files = plan.entries.size();
  std::uint64_t overall_done_bytes = 0;
  std::optional<BatchCryptoSession> batch_crypto_session;
  if (operation == FileCryptoOperation::kEncrypt) {
    auto [session_result, session] =
        BuildEncryptBatchCryptoSession(passphrase, options.security_level);
    if (!session_result.ok()) {
      batch.status = session_result;
      batch.cancelled = IsCancelledError(session_result);
      if (batch.cancelled) {
        (void)reporter.MarkCancelled();
      } else {
        (void)reporter.MarkFailed();
      }
      return batch;
    }
    batch_crypto_session = std::move(session);
  } else {
    batch_crypto_session.emplace();
  }

  for (const auto& plan_entry : plan.entries) {
    if (const auto file_select_result = reporter.SetCurrentFile(
            ToProgressDescriptor(plan_entry), overall_done_bytes);
        !file_select_result.ok()) {
      batch.status = file_select_result;
      batch.cancelled = IsCancelledError(file_select_result);
      if (batch.cancelled) {
        (void)reporter.MarkCancelled();
      } else {
        (void)reporter.MarkFailed();
      }
      return batch;
    }

    FileCryptoResult file_result{};
    BatchCryptoSession* session_ptr = batch_crypto_session.has_value()
                                          ? &batch_crypto_session.value()
                                          : nullptr;
    if (operation == FileCryptoOperation::kEncrypt) {
      file_result = EncryptFileInternal(
          plan_entry.input_path, plan_entry.output_path, passphrase,
          options.security_level, &reporter, session_ptr);
    } else {
      file_result =
          DecryptFileInternal(plan_entry.input_path, plan_entry.output_path,
                              passphrase, &reporter, session_ptr);
    }
    overall_done_bytes = reporter.CurrentOverallDoneBytes();

    if (!file_result.ok()) {
      if (IsCancelledError(file_result)) {
        batch.status = file_result;
        batch.cancelled = true;
        (void)reporter.MarkCancelled();
        return batch;
      }

      ++batch.failed_files;
      FileCryptoBatchFileError file_error{};
      file_error.input_path = plan_entry.input_path;
      file_error.output_path = plan_entry.output_path;
      file_error.error_code = file_result.error_code;
      file_error.error_message = file_result.error_message;
      batch.file_errors.push_back(std::move(file_error));
      if (batch.status.ok()) {
        batch.status = file_result;
      }
      (void)reporter.MarkFailed();

      if (!options.continue_on_error) {
        return batch;
      }
      continue;
    }

    ++batch.succeeded_files;
    overall_done_bytes = reporter.CurrentOverallDoneBytes();
  }

  if (batch.failed_files > 0) {
    if (batch.status.ok()) {
      batch.status = MakeError(
          FileCryptoError::kCryptoOperationFailed,
          "Batch operation completed with one or more file failures.");
    }
    (void)reporter.MarkFailed();
    return batch;
  }

  batch.status = {};
  if (const auto kCompleteResult = reporter.MarkCompleted();
      !kCompleteResult.ok()) {
    batch.status = kCompleteResult;
    batch.cancelled = IsCancelledError(kCompleteResult);
    return batch;
  }
  return batch;
}

}  // namespace tracer_core::infrastructure::crypto::internal

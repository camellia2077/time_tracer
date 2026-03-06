// shared/crypto_progress_json.hpp
#ifndef API_SHARED_CRYPTO_PROGRESS_JSON_HPP_
#define API_SHARED_CRYPTO_PROGRESS_JSON_HPP_

#include <string>
#include <string_view>

#include "infrastructure/crypto/file_crypto_service.hpp"
#include "nlohmann/json.hpp"

namespace tracer_core::api::shared::crypto_progress {

namespace file_crypto = tracer_core::infrastructure::crypto;
using nlohmann::json;

[[nodiscard]] inline auto ToOperationWireValue(
    file_crypto::FileCryptoOperation operation) -> std::string_view {
  switch (operation) {
    case file_crypto::FileCryptoOperation::kEncrypt:
      return "encrypt";
    case file_crypto::FileCryptoOperation::kDecrypt:
      return "decrypt";
  }
  return "unknown";
}

[[nodiscard]] inline auto ToPhaseWireValue(file_crypto::FileCryptoPhase phase)
    -> std::string_view {
  switch (phase) {
    case file_crypto::FileCryptoPhase::kScan:
      return "scan";
    case file_crypto::FileCryptoPhase::kReadInput:
      return "read_input";
    case file_crypto::FileCryptoPhase::kCompress:
      return "compress";
    case file_crypto::FileCryptoPhase::kDeriveKey:
      return "derive_key";
    case file_crypto::FileCryptoPhase::kEncrypt:
      return "encrypt";
    case file_crypto::FileCryptoPhase::kDecrypt:
      return "decrypt";
    case file_crypto::FileCryptoPhase::kDecompress:
      return "decompress";
    case file_crypto::FileCryptoPhase::kWriteOutput:
      return "write_output";
    case file_crypto::FileCryptoPhase::kCompleted:
      return "completed";
    case file_crypto::FileCryptoPhase::kCancelled:
      return "cancelled";
    case file_crypto::FileCryptoPhase::kFailed:
      return "failed";
  }
  return "unknown";
}

[[nodiscard]] inline auto BuildProgressSnapshotJson(
    const file_crypto::FileCryptoProgressSnapshot& snapshot) -> std::string {
  return json{
      {"operation", ToOperationWireValue(snapshot.operation)},
      {"phase", ToPhaseWireValue(snapshot.phase)},
      {"current_group_label", snapshot.current_group_label},
      {"group_index", snapshot.group_index},
      {"group_count", snapshot.group_count},
      {"file_index_in_group", snapshot.file_index_in_group},
      {"file_count_in_group", snapshot.file_count_in_group},
      {"current_file_index", snapshot.current_file_index},
      {"total_files", snapshot.total_files},
      {"current_file_done_bytes", snapshot.current_file_done_bytes},
      {"current_file_total_bytes", snapshot.current_file_total_bytes},
      {"overall_done_bytes", snapshot.overall_done_bytes},
      {"overall_total_bytes", snapshot.overall_total_bytes},
      {"speed_bytes_per_sec", snapshot.speed_bytes_per_sec},
      {"remaining_bytes", snapshot.remaining_bytes},
      {"eta_seconds", snapshot.eta_seconds},
      {"current_input_path", snapshot.current_input_path.string()},
      {"current_output_path", snapshot.current_output_path.string()},
      {"input_root_path", snapshot.input_root_path.string()},
      {"output_root_path", snapshot.output_root_path.string()},
  }
      .dump();
}

}  // namespace tracer_core::api::shared::crypto_progress

#endif  // API_SHARED_CRYPTO_PROGRESS_JSON_HPP_

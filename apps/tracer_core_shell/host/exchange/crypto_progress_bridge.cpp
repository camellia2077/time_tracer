#include "host/exchange/crypto_progress_bridge.hpp"

#include "application/dto/exchange_requests.hpp"
#include "nlohmann/json.hpp"

namespace tracer_core::shell::crypto_progress_bridge {

using nlohmann::json;
[[nodiscard]] auto BuildProgressSnapshotJson(
    const tracer_core::core::dto::TracerExchangeProgressSnapshot& snapshot)
    -> std::string {
  return json{
      {"operation", snapshot.is_encrypt_operation ? "encrypt" : "decrypt"},
      {"phase", snapshot.phase},
      {"current_item", snapshot.current_item},
      {"current_group_label", snapshot.current_group_label},
      {"phase_index", snapshot.phase_index},
      {"phase_count", snapshot.phase_count},
      {"done_count", snapshot.done_count},
      {"total_count", snapshot.total_count},
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

}  // namespace tracer_core::shell::crypto_progress_bridge

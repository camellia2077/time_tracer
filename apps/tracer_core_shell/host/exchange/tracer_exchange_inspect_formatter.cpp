#include "host/exchange/tracer_exchange_inspect_formatter.hpp"

#include <sstream>

namespace tracer_core::shell::tracer_exchange {

auto BuildInspectContent(
    const tracer_core::core::dto::TracerExchangeInspectResult& inspect_result)
    -> std::string {
  std::ostringstream stream;
  stream << "File: " << inspect_result.input_tracer_path.string() << '\n';
  stream << "  version: "
         << static_cast<int>(inspect_result.outer_metadata.version) << '\n';
  stream << "  kdf_id: "
         << static_cast<int>(inspect_result.outer_metadata.kdf_id) << '\n';
  stream << "  cipher_id: "
         << static_cast<int>(inspect_result.outer_metadata.cipher_id) << '\n';
  stream << "  compression_id: "
         << static_cast<int>(inspect_result.outer_metadata.compression_id)
         << '\n';
  stream << "  compression_level: "
         << static_cast<int>(inspect_result.outer_metadata.compression_level)
         << '\n';
  stream << "  ops_limit: " << inspect_result.outer_metadata.ops_limit << '\n';
  stream << "  mem_limit_kib: " << inspect_result.outer_metadata.mem_limit_kib
         << '\n';
  stream << "  plaintext_size: " << inspect_result.outer_metadata.plaintext_size
         << '\n';
  stream << "  ciphertext_size: "
         << inspect_result.outer_metadata.ciphertext_size << '\n';
  stream << "Package:\n";
  stream << "  package_type: " << inspect_result.package_type << '\n';
  stream << "  package_version: " << inspect_result.package_version << '\n';
  stream << "  producer_platform: " << inspect_result.producer_platform << '\n';
  stream << "  producer_app: " << inspect_result.producer_app << '\n';
  stream << "  created_at_utc: " << inspect_result.created_at_utc << '\n';
  stream << "  source_root_name: " << inspect_result.source_root_name << '\n';
  stream << "  payload_file_count: " << inspect_result.payload_file_count
         << '\n';
  for (const auto& entry : inspect_result.payload_entries) {
    stream << "  " << entry.relative_path << ": "
           << (entry.present ? "present" : "missing") << " ("
           << entry.size_bytes << " bytes)\n";
  }
  for (const auto& entry : inspect_result.converter_entries) {
    stream << "  " << entry.relative_path << ": "
           << (entry.present ? "present" : "missing") << " ("
           << entry.size_bytes << " bytes)\n";
  }
  return stream.str();
}

}  // namespace tracer_core::shell::tracer_exchange

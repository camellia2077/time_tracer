module;

#include "infrastructure/io/core/file_writer.hpp"

export module tracer.adapters.io.core.writer;

export namespace tracer::adapters::io::modcore {

using Writer = ::FileWriter;

inline auto WriteBytes(const std::filesystem::path& path,
                       std::span<const std::uint8_t> bytes) -> void {
  Writer::WriteBytes(path, bytes);
}

inline auto WriteCanonicalText(const std::filesystem::path& path,
                               std::string_view content) -> void {
  Writer::WriteCanonicalText(path, content);
}

}  // namespace tracer::adapters::io::modcore

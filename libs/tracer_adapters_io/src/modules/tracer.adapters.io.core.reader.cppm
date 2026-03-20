module;

#include "infrastructure/io/core/file_reader.hpp"

export module tracer.adapters.io.core.reader;

export namespace tracer::adapters::io::modcore {

using Reader = ::FileReader;

[[nodiscard]] inline auto ReadBytes(const std::filesystem::path& path)
    -> std::vector<std::uint8_t> {
  return Reader::ReadBytes(path);
}

[[nodiscard]] inline auto ReadCanonicalText(const std::filesystem::path& path)
    -> std::string {
  return Reader::ReadCanonicalText(path);
}

}  // namespace tracer::adapters::io::modcore

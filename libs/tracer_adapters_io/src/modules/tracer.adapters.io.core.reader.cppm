module;

#include "infrastructure/io/core/file_reader.hpp"

export module tracer.adapters.io.core.reader;

export namespace tracer::adapters::io::modcore {

using Reader = ::FileReader;

[[nodiscard]] inline auto ReadContent(const std::filesystem::path& path)
    -> std::string {
  return Reader::ReadContent(path);
}

}  // namespace tracer::adapters::io::modcore

module;

#include "infrastructure/io/core/file_writer.hpp"

export module tracer.adapters.io.core.writer;

export namespace tracer::adapters::io::modcore {

using Writer = ::FileWriter;

inline auto WriteContent(const std::filesystem::path& path,
                         const std::string& content) -> void {
  Writer::WriteContent(path, content);
}

}  // namespace tracer::adapters::io::modcore

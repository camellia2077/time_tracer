// infrastructure/io/txt_ingest_input_provider.cpp
#if TT_ENABLE_CPP20_MODULES
import tracer.adapters.io.core.fs;
import tracer.adapters.io.core.reader;
import tracer.adapters.io.utils.file_utils;
#endif

#include "infrastructure/io/txt_ingest_input_provider.hpp"

#include <algorithm>
#include <string>
#include <utility>

#if !TT_ENABLE_CPP20_MODULES
#include "infrastructure/io/core/file_reader.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/utils/file_utils.hpp"
#endif

#if TT_ENABLE_CPP20_MODULES
namespace modcore = tracer::adapters::io::modcore;
namespace modutils = tracer::adapters::io::modutils;
#else
namespace modcore {

using FileSystem = ::FileSystemHelper;
using Reader = ::FileReader;

[[nodiscard]] inline auto Exists(const std::filesystem::path& path) -> bool {
  return FileSystem::Exists(path);
}

[[nodiscard]] inline auto IsRegularFile(const std::filesystem::path& path)
    -> bool {
  return FileSystem::IsRegularFile(path);
}

[[nodiscard]] inline auto ReadContent(const std::filesystem::path& path)
    -> std::string {
  return Reader::ReadContent(path);
}

}  // namespace modcore

namespace modutils {

[[nodiscard]] inline auto FindFilesByExtensionRecursively(
    const std::filesystem::path& root_path, const std::string& extension)
    -> std::vector<std::filesystem::path> {
  return ::FileUtils::FindFilesByExtensionRecursively(root_path, extension);
}

}  // namespace modutils
#endif

namespace infrastructure::io {

auto TxtIngestInputProvider::CollectTextInputs(
    const std::filesystem::path& input_root, std::string_view extension) const
    -> tracer_core::application::dto::IngestInputCollection {
  tracer_core::application::dto::IngestInputCollection collection;
  collection.input_exists = modcore::Exists(input_root);
  if (!collection.input_exists) {
    return collection;
  }

  const std::string kExtension(extension);
  std::vector<std::filesystem::path> files =
      modutils::FindFilesByExtensionRecursively(input_root, kExtension);
  if (modcore::IsRegularFile(input_root) &&
      input_root.extension() == kExtension &&
      std::ranges::find(files, input_root) == files.end()) {
    files.push_back(input_root);
  }

  collection.inputs.reserve(files.size());
  for (const auto& file_path : files) {
    std::string source_label = file_path.filename().string();
    if (source_label.empty()) {
      source_label = file_path.string();
    }
    collection.inputs.push_back(
        {.source_id = file_path.string(),
         .source_label = std::move(source_label),
         .content = modcore::ReadContent(file_path)});
  }

  return collection;
}

}  // namespace infrastructure::io

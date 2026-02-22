// infrastructure/io/txt_ingest_input_provider.cpp
#include "infrastructure/io/txt_ingest_input_provider.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "infrastructure/io/core/file_reader.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/utils/file_utils.hpp"

namespace infrastructure::io {

auto TxtIngestInputProvider::CollectTextInputs(
    const std::filesystem::path& input_root, std::string_view extension) const
    -> time_tracer::application::dto::IngestInputCollection {
  time_tracer::application::dto::IngestInputCollection collection;
  collection.input_exists = FileSystemHelper::Exists(input_root);
  if (!collection.input_exists) {
    return collection;
  }

  const std::string kExtension(extension);
  std::vector<std::filesystem::path> files =
      FileUtils::FindFilesByExtensionRecursively(input_root, kExtension);
  if (FileSystemHelper::IsRegularFile(input_root) &&
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
         .content = FileReader::ReadContent(file_path)});
  }

  return collection;
}

}  // namespace infrastructure::io

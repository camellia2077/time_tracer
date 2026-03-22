// infra/io/processed_data_io.cpp
#include "infra/io/internal/runtime_adapter_types.hpp"

#include <exception>
#include <iterator>
#include <nlohmann/json.hpp>

#include "infra/io/file_import_reader.hpp"
#include "infra/io/processed_json_validation.hpp"
#include "infra/serialization/json_serializer.hpp"

import tracer.adapters.io.core.fs;
import tracer.adapters.io.core.writer;
import tracer.core.domain.ports.diagnostics;

namespace modcore = tracer::adapters::io::modcore;
namespace modports = tracer::core::domain::modports;

namespace fs = std::filesystem;

namespace infrastructure::io::internal {

auto ProcessedDataLoaderAdapter::LoadDailyLogs(const std::string& processed_path)
    -> tracer_core::application::ports::ProcessedDataLoadResult {
  tracer_core::application::ports::ProcessedDataLoadResult result;

  auto import_payload = FileImportReader::ReadJsonFiles(processed_path);
  for (const auto& [filepath, content] : import_payload) {
    try {
      auto json_obj = nlohmann::json::parse(content);
      auto validation_input = BuildProcessedJsonValidationInput(json_obj);
      auto validation_errors =
          CollectProcessedJsonValidationErrors(filepath, validation_input);
      if (!validation_errors.empty()) {
        result.errors.insert(result.errors.end(),
                             std::make_move_iterator(validation_errors.begin()),
                             std::make_move_iterator(validation_errors.end()));
        continue;
      }
      auto logs = serializer::JsonSerializer::DeserializeDays(content);
      result.data_by_source[filepath] = std::move(logs);
    } catch (const std::exception& e) {
      result.errors.push_back({.source = filepath, .message = e.what()});
    }
  }

  return result;
}

auto ProcessedDataWriter::Write(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const fs::path& output_root) -> std::vector<fs::path> {
  std::vector<fs::path> written_files;

  for (const auto& [month_key, month_days] : data) {
    fs::path output_file_path = output_root / "data" / (month_key + ".json");
    fs::path month_output_dir = output_file_path.parent_path();

    try {
      modcore::CreateDirectories(month_output_dir);

      modcore::WriteCanonicalText(
          output_file_path, serializer::JsonSerializer::SerializeDays(month_days, 4));

      written_files.push_back(output_file_path);
    } catch (const std::exception& e) {
      modports::EmitError(
          "错误: 无法写入输出文件: " + output_file_path.string() + " - " +
          e.what());
    }
  }
  return written_files;
}

auto ProcessedDataStorageAdapter::WriteProcessedData(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::filesystem::path& output_root)
    -> std::vector<std::filesystem::path> {
  return ProcessedDataWriter::Write(data, output_root);
}

}  // namespace infrastructure::io::internal

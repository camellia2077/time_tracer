// infrastructure/io/processed_data_io.cpp
#include "infrastructure/io/processed_data_io.hpp"

#include <exception>
#include <nlohmann/json.hpp>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/core/file_writer.hpp"
#include "infrastructure/io/file_import_reader.hpp"
#include "infrastructure/serialization/json_serializer.hpp"
#include "shared/types/ansi_colors.hpp"

namespace fs = std::filesystem;

namespace infrastructure::io {

auto ProcessedDataLoader::LoadDailyLogs(const std::string& processed_path)
    -> tracer_core::application::ports::ProcessedDataLoadResult {
  tracer_core::application::ports::ProcessedDataLoadResult result;

  auto import_payload = FileImportReader::ReadJsonFiles(processed_path);
  for (const auto& [filepath, content] : import_payload) {
    try {
      auto json_obj = nlohmann::json::parse(content);
      auto logs = serializer::JsonSerializer::DeserializeDays(json_obj);
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
      FileSystemHelper::CreateDirectories(month_output_dir);

      nlohmann::json json_content =
          serializer::JsonSerializer::SerializeDays(month_days);
      FileWriter::WriteContent(output_file_path, json_content.dump(4));

      written_files.push_back(output_file_path);
    } catch (const std::exception& e) {
      tracer_core::domain::ports::EmitError(
          std::string(tracer_core::common::colors::kRed) +
          "错误: 无法写入输出文件: " + output_file_path.string() + " - " +
          e.what() + std::string(tracer_core::common::colors::kReset));
    }
  }
  return written_files;
}

auto ProcessedDataStorage::WriteProcessedData(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::filesystem::path& output_root)
    -> std::vector<std::filesystem::path> {
  return ProcessedDataWriter::Write(data, output_root);
}

}  // namespace infrastructure::io

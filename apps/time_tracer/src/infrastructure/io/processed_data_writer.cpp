// infrastructure/io/processed_data_writer.cpp
#include "infrastructure/io/processed_data_writer.hpp"

#include <iostream>

#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/core/file_writer.hpp"
#include "infrastructure/serialization/json_serializer.hpp"
#include "shared/types/ansi_colors.hpp"

namespace fs = std::filesystem;

namespace infrastructure::io {

auto ProcessedDataWriter::Write(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::map<std::string, nlohmann::json>& cached_json_outputs,
    const fs::path& output_root) -> std::vector<fs::path> {
  std::vector<fs::path> written_files;

  for (const auto& [month_key, month_days] : data) {
    fs::path output_file_path = output_root / "data" / (month_key + ".json");
    fs::path month_output_dir = output_file_path.parent_path();

    try {
      FileSystemHelper::CreateDirectories(month_output_dir);

      std::string content_to_write;

      // [优化] 优先使用缓存的 JSON 对象
      auto cache_it = cached_json_outputs.find(month_key);
      if (cache_it != cached_json_outputs.end()) {
        content_to_write = cache_it->second.dump(4);
      } else {
        // 如果没有缓存（比如跳过了验证步骤），则执行序列化
        nlohmann::json json_content =
            serializer::JsonSerializer::SerializeDays(month_days);
        content_to_write = json_content.dump(4);
      }

      FileWriter::WriteContent(output_file_path, content_to_write);

      written_files.push_back(output_file_path);
    } catch (const std::exception& e) {
      std::cerr << time_tracer::common::colors::kRed
                << "错误: 无法写入输出文件: " << output_file_path << " - "
                << e.what() << time_tracer::common::colors::kReset << std::endl;
    }
  }
  return written_files;
}

}  // namespace infrastructure::io

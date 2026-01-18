// core/file/utils/processed_data_writer.cpp
#include "processed_data_writer.hpp"
#include "serializer/json_serializer.hpp"
#include "io/core/file_system_helper.hpp"
#include "io/core/file_writer.hpp"
#include "common/ansi_colors.hpp"
#include <iostream>

namespace fs = std::filesystem;

std::vector<fs::path> ProcessedDataWriter::write(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const fs::path& output_root
) {
    std::vector<fs::path> written_files;

    for (const auto& [month_key, month_days] : data) {
        // 构建路径: output/Processed_Date/YYYY-MM.json
        fs::path output_file_path = output_root / "Processed_Date" / (month_key + ".json");
        fs::path month_output_dir = output_file_path.parent_path();

        try {
            // 1. IO: 准备目录
            FileSystemHelper::create_directories(month_output_dir);
            
            // 2. Serializer: Struct -> JSON
            nlohmann::json json_content = serializer::JsonSerializer::serializeDays(month_days);
            
            // 3. IO: 写入文件
            // 使用 dump(4) 生成格式化的 JSON 字符串
            FileWriter::write_content(output_file_path, json_content.dump(4));
            
            written_files.push_back(output_file_path);
        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "错误: 无法写入输出文件: " << output_file_path 
                       << " - " << e.what() << RESET_COLOR << std::endl;
             // 继续处理下一个月的数据
        }
    }
    return written_files;
}
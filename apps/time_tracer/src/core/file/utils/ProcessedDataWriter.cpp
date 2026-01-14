// core/file/utils/ProcessedDataWriter.cpp
#include "ProcessedDataWriter.hpp"
#include "converter/convert/io/JsonWriter.hpp"
#include "io/core/FileSystemHelper.hpp"
#include "io/core/FileWriter.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

std::vector<fs::path> ProcessedDataWriter::write(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const fs::path& output_root,
    const ConverterConfig& config
) {
    std::vector<fs::path> written_files;
    JsonWriter generator;

    for (const auto& [month_key, month_days] : data) {
        fs::path output_file_path = output_root / "Processed_Date" / (month_key + ".json");
        fs::path month_output_dir = output_file_path.parent_path();

        try {
            FileSystemHelper::create_directories(month_output_dir);
            
            std::stringstream buffer;
            generator.write(buffer, month_days, config); 
            
            FileWriter::write_content(output_file_path, buffer.str());
            
            written_files.push_back(output_file_path);
        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "错误: 无法写入输出文件: " << output_file_path 
                       << " - " << e.what() << RESET_COLOR << std::endl;
             // 继续处理下一个月的数据，不中断整个流程
        }
    }
    return written_files;
}
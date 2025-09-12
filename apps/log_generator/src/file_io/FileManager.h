#pragma once
#include <string>
#include <filesystem>

class FileManager {
public:
    // 准备所有需要的输出目录
    bool setup_directories(const std::string& master_dir, int start_year, int end_year);
    
    // 将内容写入指定的日志文件
    bool write_log_file(const std::filesystem::path& file_path, const std::string& content);
};
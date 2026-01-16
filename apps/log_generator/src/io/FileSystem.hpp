// io/FileSystem.hpp
#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP

#include <string>
#include <filesystem>
#include <optional>

class FileSystem {
public:
    virtual ~FileSystem() = default;
    virtual bool setup_directories(const std::string& master_dir, int start_year, int end_year) = 0;
    virtual bool write_log_file(const std::filesystem::path& file_path, const std::string& content) = 0;
    virtual std::optional<std::string> read_file(const std::filesystem::path& file_path) = 0;
};

#endif // FILE_SYSTEM_HPP
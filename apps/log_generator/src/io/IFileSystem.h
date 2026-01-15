#ifndef IO_IFILESYSTEM_H
#define IO_IFILESYSTEM_H

#include <string>
#include <filesystem>
#include <optional>

class IFileSystem {
public:
    virtual ~IFileSystem() = default;
    virtual bool setup_directories(const std::string& master_dir, int start_year, int end_year) = 0;
    virtual bool write_log_file(const std::filesystem::path& file_path, const std::string& content) = 0;
    virtual std::optional<std::string> read_file(const std::filesystem::path& file_path) = 0;
};

#endif // IO_IFILESYSTEM_H
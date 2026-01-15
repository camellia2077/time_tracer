
#include "IFileSystem.h" // 引入接口
#include <string>
#include <filesystem>
#include <optional>

// [修改] 继承自 IFileSystem
class FileManager : public IFileSystem {
public:
    bool setup_directories(const std::string& master_dir, int start_year, int end_year) override;
    
    bool write_log_file(const std::filesystem::path& file_path, const std::string& content) override;

    std::optional<std::string> read_file(const std::filesystem::path& file_path) override;
};
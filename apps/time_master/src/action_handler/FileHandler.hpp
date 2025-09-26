// action_handler/FileHandler.hpp
#ifndef FILE_PROCESSING_HANDLER_HPP
#define FILE_PROCESSING_HANDLER_HPP

#include <string>
#include <filesystem>
#include "common/AppConfig.hpp" // For AppConfig and AppOptions

namespace fs = std::filesystem;

class FileHandler {
public:
    FileHandler(const std::string& db_path, const AppConfig& config, const std::string& main_config_path, const fs::path& output_root_path);
    ~FileHandler();

    void run_preprocessing(const std::string& input_path, const AppOptions& options);
    void run_database_import(const std::string& processed_path_str);
    void run_full_pipeline_and_import(const std::string& source_path);
    const AppConfig& get_config() const;

private:
    const AppConfig& app_config_;
    std::string main_config_path_;
    std::string db_path_;
    fs::path output_root_path_;
};

#endif // FILE_PROCESSING_HANDLER_HPP
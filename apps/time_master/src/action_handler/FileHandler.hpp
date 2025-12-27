// action_handler/FileHandler.hpp
#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include <string>
#include <filesystem>
#include "common/AppConfig.hpp" 
// [新增] 引入 ValidatorUtils
#include "reprocessing/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

class FileHandler {
public:
    FileHandler(const std::string& db_path, const AppConfig& config, const std::string& main_config_path, const fs::path& output_root_path);
    ~FileHandler();

    void run_preprocessing(const std::string& input_path, const AppOptions& options);
    void run_database_import(const std::string& processed_path_str);
    
    // [核心修改] 增加 DateCheckMode 参数
    void run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode);
    
    const AppConfig& get_config() const;

private:
    const AppConfig& app_config_;
    std::string main_config_path_;
    std::string db_path_;
    fs::path output_root_path_;
};

#endif // FILE_HANDLER_HPP

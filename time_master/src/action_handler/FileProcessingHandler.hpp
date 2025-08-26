#ifndef FILE_PROCESSING_HANDLER_HPP
#define FILE_PROCESSING_HANDLER_HPP

#include <string>
#include <memory>
#include "common/AppConfig.hpp"

namespace fs = std::filesystem;

// [新增] 定义用于预处理命令的选项结构体
struct PreprocessingOptions {
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_check = false;
};

class FileProcessingHandler {
public:
    FileProcessingHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~FileProcessingHandler();

    // [新增] 声明新的预处理方法
    void run_preprocessing(const std::string& input_path, const PreprocessingOptions& options);
    
    void run_database_import(const std::string& processed_path);
    void run_full_pipeline_and_import(const std::string& source_path);
    const AppConfig& get_config() const;

private:
    AppConfig app_config_;
    std::string main_config_path_;
    std::string db_name_;
};

#endif // FILE_PROCESSING_HANDLER_HPP
#ifndef FILE_PROCESSING_HANDLER_H
#define FILE_PROCESSING_HANDLER_H

#include <string>
#include <memory>
#include "common/AppConfig.h"

namespace fs = std::filesystem;

class FileProcessingHandler {
public:
    FileProcessingHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~FileProcessingHandler();

    void run_database_import(const std::string& processed_path);
    void run_full_pipeline_and_import(const std::string& source_path);
    const AppConfig& get_config() const;

private:
    AppConfig app_config_;
    std::string main_config_path_;
    std::string db_name_;
};

#endif // FILE_PROCESSING_HANDLER_H
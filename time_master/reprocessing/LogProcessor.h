// --- START OF FILE reprocessing/LogProcessor.h ---

#ifndef LOG_PROCESSOR_H
#define LOG_PROCESSOR_H

#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp> 

struct AppConfig {
    std::string interval_processor_config_path;
    std::string format_validator_config_path;
    std::string error_log_path;
};


struct AppOptions {
    std::string input_path;
    bool run_all = false; // 此标志现在由 ActionHandler 解释
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
};

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    /**
     * @brief 对单个文件执行指定的处理操作（验证、转换）。
     * * @param source_file 源文件路径。
     * @param output_file 转换后的目标文件路径（如果 convert 为 true）。
     * @param options 包含执行哪些操作（校验源、转换、校验目标）的选项。
     * @return true 如果所有请求的操作都成功。
     * @return false 如果任何操作失败。
     */
    bool processFile(const std::filesystem::path& source_file, 
                     const std::filesystem::path& output_file, 
                     const AppOptions& options);
    
    // --- [NEW PUBLIC UTILITY] ---
    /**
     * @brief 收集指定路径下的所有待处理 .txt 文件。
     * * @param input_path 输入路径（文件或目录）。
     * @param out_files [out] 收集到的文件路径列表。
     * @return true 如果路径有效。
     * @return false 如果路径不存在。
     */
    bool collectFilesToProcess(const std::string& input_path, std::vector<std::filesystem::path>& out_files);


private:
    AppConfig config_;

    std::string extractYearFromPath(const std::filesystem::path& file_path);
};

#endif // LOG_PROCESSOR_H
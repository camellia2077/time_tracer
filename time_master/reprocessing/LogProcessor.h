// reprocessing/LogProcessor.cpp
#ifndef LOG_PROCESSOR_H
#define LOG_PROCESSOR_H

#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp> 
#include <optional> // 为了使用 std::optional

struct AppConfig {
    std::string interval_processor_config_path;
    std::string format_validator_config_path;
    std::string error_log_path;
    std::optional<std::string> export_path;// 用于存储从 config.json 中读取的自定义导出根目录路径
};

struct AppOptions {
    std::string input_path;
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
};

struct ProcessingTimings {
    double validation_source_ms = 0.0;
    double conversion_ms = 0.0;
    double validation_output_ms = 0.0;
};

struct ProcessingResult {
    bool success = true;
    ProcessingTimings timings;
};

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    /**
     * @brief 对单个文件执行指定的处理操作（验证、转换）。
     * * @param source_file 源文件路径。
     * @param output_file 转换后的目标文件路径（如果 convert 为 true）。
     * @param options 包含执行哪些操作（校验源、转换、校验目标）的选项。
     * @return 一个 ProcessingResult 结构体，包含操作是否成功以及各阶段的耗时。
     */
    ProcessingResult processFile(const std::filesystem::path& source_file, 
                                 const std::filesystem::path& output_file, 
                                 const AppOptions& options);
    
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
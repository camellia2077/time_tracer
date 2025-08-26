// time_master/reprocessing/LogProcessor.hpp
#ifndef LOG_PROCESSOR_HPP
#define LOG_PROCESSOR_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp" // 引入新的通用配置头文件

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    /**
     * @brief 对单个文件执行指定的处理操作（验证、转换）。
     * @param source_file 源文件路径。
     * @param output_file 转换后的目标文件路径（如果 convert 为 true）。
     * @param options 包含执行哪些操作（校验源、转换、校验目标）的选项。
     * @return 一个 ProcessingResult 结构体，包含操作是否成功以及各阶段的耗时。
     */
    ProcessingResult processFile(const std::filesystem::path& source_file, 
                                 const std::filesystem::path& output_file, 
                                 const AppOptions& options);
    
    /**
     * @brief 收集指定路径下的所有待处理 .txt 文件。
     * @param input_path 输入路径（文件或目录）。
     * @param out_files [out] 收集到的文件路径列表。
     * @return true 如果路径有效。
     * @return false 如果路径不存在。
     */
    bool collectFilesToProcess(const std::string& input_path, std::vector<std::filesystem::path>& out_files);


private:
    AppConfig config_;

    std::string extractYearFromPath(const std::filesystem::path& file_path);
};

#endif // LOG_PROCESSOR_HPP
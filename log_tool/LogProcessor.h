// LogProcessor.h

#ifndef LogProcessor_H
#define LogProcessor_H

#include <string>
#include <vector>
#include <filesystem>

// 配置：程序启动时设置，通常不变
struct AppConfig {
    std::string interval_config_path;
    std::string validator_config_path;
    std::string error_log_path;
};

// 选项：每次运行时根据用户输入决定
struct AppOptions {
    std::string input_path;
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
};

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);

    /**
     * @brief 运行核心业务流程。
     * @param options 用户指定的运行选项。
     * @return 如果所有请求的操作都成功，则返回 true；否则返回 false。
     */
    bool run(const AppOptions& options);

private:
    AppConfig config_;
    AppOptions options_; // 存储当前运行的选项

    // 从 main.cpp 迁移过来的辅助函数
    bool collectFilesToProcess(std::vector<std::filesystem::path>& out_files);
    std::string extractYearFromPath(const std::filesystem::path& file_path);
    void printSummary() const;

    // 核心业务流程
    bool runAllInOneMode();
    bool runIndividualMode();

    // 状态追踪
    int v_source_success_ = 0, v_source_fail_ = 0;
    int convert_success_ = 0, convert_fail_ = 0;
    int v_output_success_ = 0, v_output_fail_ = 0;
};

#endif // LogProcessor_H
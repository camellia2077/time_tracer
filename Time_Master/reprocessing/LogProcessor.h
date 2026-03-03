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
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
};

class LogProcessor {
public:
    explicit LogProcessor(const AppConfig& config);
    bool run(const AppOptions& options);

private:
    AppConfig config_;
    AppOptions options_;

    bool collectFilesToProcess(std::vector<std::filesystem::path>& out_files);
    std::string extractYearFromPath(const std::filesystem::path& file_path);
    void printSummary() const;

    bool runAllInOneMode();
    bool runIndividualMode();

    int v_source_success_ = 0, v_source_fail_ = 0;
    int convert_success_ = 0, convert_fail_ = 0;
    int v_output_success_ = 0, v_output_fail_ = 0;
};

#endif // LOG_PROCESSOR_H
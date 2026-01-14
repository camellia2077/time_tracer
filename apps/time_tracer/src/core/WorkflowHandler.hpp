// core/WorkflowHandler.hpp
#ifndef WORKFLOW_HANDLER_HPP
#define WORKFLOW_HANDLER_HPP

#include <string>
#include <filesystem>
#include <map>
#include <vector>

// [修复] 更新包含路径
#include "common/config/AppConfig.hpp"
// [修复] 新增 AppOptions 引用，因为 run_converter 方法使用了它
#include "common/AppOptions.hpp"

#include "converter/validator/common/ValidatorUtils.hpp"
#include "common/model/DailyLog.hpp" 

namespace fs = std::filesystem;

class WorkflowHandler {
public:
    WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path);
    ~WorkflowHandler();

    // 运行预处理流水线
    void run_converter(const std::string& input_path, const AppOptions& options);
    // 传统的基于文件的导入 (现在只负责编排，不再负责 IO)
    void run_database_import(const std::string& processed_path_str);
    // 基于内存数据的导入
    void run_database_import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map);
    // 完整流程
    void run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed = false);
    const AppConfig& get_config() const;

private:
    const AppConfig& app_config_;
    std::string db_path_;
    fs::path output_root_path_;
};

#endif // WORKFLOW_HANDLER_HPP

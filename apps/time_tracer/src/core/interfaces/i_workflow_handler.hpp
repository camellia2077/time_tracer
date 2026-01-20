// core/interfaces/i_workflow_handler.hpp
#ifndef CORE_INTERFACES_I_WORKFLOW_HANDLER_HPP_
#define CORE_INTERFACES_I_WORKFLOW_HANDLER_HPP_

#include <string>
#include <map>
#include <vector>
#include "common/app_options.hpp"
#include "common/model/daily_log.hpp"
#include "validator/common/ValidatorUtils.hpp"

// 前向声明，减少头文件依赖
class AppConfig;

class IWorkflowHandler {
public:
    virtual ~IWorkflowHandler() = default;

    // 运行预处理流水线
    virtual void run_converter(const std::string& input_path, const AppOptions& options) = 0;
    
    // 传统的基于文件的导入
    virtual void run_database_import(const std::string& processed_path_str) = 0;
    
    // 基于内存数据的导入
    virtual void run_database_import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map) = 0;
    
    // 完整流程 (Ingest/Blink)
    virtual void run_ingest(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed = false) = 0;
    
    // 获取配置
    virtual const AppConfig& get_config() const = 0;
};

#endif // CORE_INTERFACES_I_WORKFLOW_HANDLER_HPP_
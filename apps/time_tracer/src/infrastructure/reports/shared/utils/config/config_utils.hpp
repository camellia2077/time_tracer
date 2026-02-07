// infrastructure/reports/shared/utils/config/config_utils.hpp
#ifndef REPORTS_SHARED_UTILS_CONFIG_CONFIG_UTILS_H_
#define REPORTS_SHARED_UTILS_CONFIG_CONFIG_UTILS_H_

#include <filesystem>
#include <nlohmann/json.hpp>

/**
 * @brief 从指定路径加载并解析 JSON 配置文件。
 * @param config_path 配置文件的路径。
 * @param error_message_prefix 当文件无法打开时，用于构造异常信息的前缀。
 * @return 解析后的 nlohmann::json 对象。
 * @throws std::runtime_error 如果无法打开或解析文件。
 */
nlohmann::json load_json_config(const std::filesystem::path& config_path,
                                const std::string& error_message_prefix);

#endif  // REPORTS_SHARED_UTILS_CONFIG_CONFIG_UTILS_H_
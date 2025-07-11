#ifndef FILE_VALIDATOR_H
#define FILE_VALIDATOR_H

#include "ValidatorUtils.h"
#include <string>
#include <set>

// 定义一个枚举来区分要执行的验证类型
enum class ValidatorType {
    Source,
    Output
};

/**
 * @class FileValidator
 * @brief 一个外观类，为不同类型的文件验证提供统一的接口。
 *
 * 这个类封装了 SourceFileValidator 和 OutputFileValidator 的复杂性，
 * 允许客户端通过一个简单的接口来验证源文件或转换后的输出文件。
 */
class FileValidator {
public:
    /**
     * @brief 构造函数，初始化所有验证器可能需要的配置。
     * @param source_config_path 用于 SourceFileValidator 的配置文件路径。
     * @param output_validator_config_path 用于 OutputFileValidator 的活动类别配置文件路径。
     * @param output_header_config_path 用于 OutputFileValidator 的头部顺序配置文件路径。
     */
    FileValidator(const std::string& source_config_path, 
                  const std::string& output_validator_config_path, 
                  const std::string& output_header_config_path);

    /**
     * @brief 执行文件验证的核心函数。
     * @param file_path 要验证的文件的路径。
     * @param type 要执行的验证类型 (Source 或 Output)。
     * @param errors 用于收集验证错误的集合。
     * @param enable_day_count_check (仅用于Output类型) 是否启用月份天数连续性检查。
     * @return 如果没有发现错误，则返回 true；否则返回 false。
     */
    bool validate(const std::string& file_path, 
                  ValidatorType type, 
                  std::set<Error>& errors, 
                  bool enable_day_count_check_for_output = false);

private:
    // 存储所有需要的配置路径
    std::string source_config_path_;
    std::string output_validator_config_path_;
    std::string output_header_config_path_;
};

#endif // FILE_VALIDATOR_H
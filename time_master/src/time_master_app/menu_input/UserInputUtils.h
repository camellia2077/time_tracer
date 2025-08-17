// common/UserInputUtils.h
#ifndef USER_INPUT_UTILS_H
#define USER_INPUT_UTILS_H

#include <string>
#include <vector>
#include "queries/shared/ReportFormat.h"

namespace UserInputUtils {

    /**
     * @brief 提示用户输入一个有效的文件或目录路径。
     * @param prompt_message 显示给用户的提示信息。
     * @return 如果路径有效则返回路径字符串，否则返回空字符串。
     */
    std::string get_valid_path_input(const std::string& prompt_message);

    /**
     * @brief 提示用户输入一个有效的日期 (YYYYMMDD)。
     * @return 返回格式化的日期字符串，如果用户中止则返回空字符串。
     */
    std::string get_valid_date_input();

    /**
     * @brief 提示用户输入一个有效的月份 (YYYYMM)。
     * @return 返回格式化的月份字符串，如果用户中止则返回空字符串。
     */
    std::string get_valid_month_input();

    /**
     * @brief 提示用户选择报告格式 (Markdown 或 TeX)。
     * @return 返回用户选择的 ReportFormat 枚举值。
     */
    ReportFormat get_report_format_from_user();

    /**
     * @brief 提示用户输入一个或多个以逗号分隔的数字。
     * @param prompt_message 显示给用户的提示信息。
     * @return 返回一个包含所有有效数字的整数向量。
     */
    std::vector<int> get_integer_list_input(const std::string& prompt_message);

} // namespace UserInputUtils

#endif // USER_INPUT_UTILS_H
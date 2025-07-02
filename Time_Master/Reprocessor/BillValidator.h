#ifndef BILL_VALIDATOR_H
#define BILL_VALIDATOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <nlohmann/json.hpp>

// 使用 nlohmann::json 的命名空间
using json = nlohmann::json;

class BillValidator {
public:
    /**
     * @brief 构造函数，加载并解析配置文件。
     * @param config_path JSON 配置文件的路径。
     * @throws std::runtime_error 如果配置文件加载或解析失败。
     */
    BillValidator(const std::string& config_path);

    /**
     * @brief 验证给定的账单文件。
     * @param bill_file_path 要验证的账单文件的路径。
     * @return 如果没有发现任何“错误”，则返回 true，否则返回 false。警告不会导致返回 false。
     */
    bool validate(const std::string& bill_file_path);

private:
    // --- 配置处理 ---
    json config_data;
    std::unordered_map<std::string, std::set<std::string>> validation_map;
    std::set<std::string> all_parent_titles;

    void _load_config(const std::string& config_path);
    void _transform_config_for_validation();

    // --- 状态与数据跟踪 ---
    enum class State {
        EXPECT_PARENT,
        EXPECT_SUB,
        EXPECT_CONTENT
    };
    
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    // 用于后期验证的数据结构：父标题 -> (子标题 -> 内容行数)
    std::unordered_map<std::string, std::unordered_map<std::string, int>> bill_structure;

    // --- 验证步骤 ---
    void _reset_state();
    bool _validate_date_and_remark(std::ifstream& file, int& line_num);
    void _process_line(const std::string& line, int line_num, State& current_state, std::string& current_parent, std::string& current_sub);
    void _post_validation_checks();

    // --- 状态处理函数 ---
    void _handle_parent_state(const std::string& line, int line_num, State& current_state, std::string& current_parent);
    void _handle_sub_state(const std::string& line, int line_num, State& current_state, const std::string& current_parent, std::string& current_sub);
    void _handle_content_state(const std::string& line, int line_num, State& current_state, std::string& current_parent, std::string& current_sub);
};

#endif // BILL_VALIDATOR_H
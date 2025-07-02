#include "BillValidator.h"
#include <fstream>
#include <iostream>
#include <regex>

// --- 构造与配置加载 ---

BillValidator::BillValidator(const std::string& config_path) {
    _load_config(config_path);
    _transform_config_for_validation();
}

void BillValidator::_load_config(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw std::runtime_error("错误: 无法打开配置文件 '" + config_path + "'");
    }
    try {
        file >> config_data;
    } catch (json::parse_error& e) {
        throw std::runtime_error("错误: 解析 JSON 配置文件失败: " + std::string(e.what()));
    }
}

void BillValidator::_transform_config_for_validation() {
    if (!config_data.contains("categories") || !config_data["categories"].is_array() || config_data["categories"].empty()) {
        throw std::runtime_error("错误: 配置文件格式不正确或 'categories' 列表为空。");
    }

    for (const auto& category : config_data["categories"]) {
        // *** 修改点 ***
        std::string parent_title = category["parent_item"]; 
        all_parent_titles.insert(parent_title);

        std::set<std::string> sub_titles;
        // *** 修改点 ***
        for (const auto& sub : category["sub_items"]) { 
            sub_titles.insert(sub.get<std::string>());
        }
        validation_map[parent_title] = sub_titles;
    }
}


// --- 主验证流程 ---

void BillValidator::_reset_state() {
    errors.clear();
    warnings.clear();
    bill_structure.clear();
}

bool BillValidator::validate(const std::string& bill_file_path) {
    _reset_state();

    std::ifstream file(bill_file_path);
    if (!file.is_open()) {
        errors.push_back("严重错误: 无法打开账单文件 '" + bill_file_path + "'");
        std::cerr << errors.back() << std::endl;
        return false;
    }

    int line_num = 0;
    if (!_validate_date_and_remark(file, line_num)) {
        // 错误已在函数内添加
    } else {
        State current_state = State::EXPECT_PARENT;
        std::string current_parent;
        std::string current_sub;
        std::string line;

        // 在读取主要内容前，检查上一个子标题是否有内容
        // 这处理了最后一个子标题没有内容的情况
        if (!current_sub.empty() && bill_structure[current_parent][current_sub] == 0) {
            warnings.push_back("警告 (文件结尾): 子标题 '" + current_sub + "' 缺少内容行。");
        }

        while (std::getline(file, line)) {
            line_num++;
            if (line.empty()) continue; // 跳过空行
            _process_line(line, line_num, current_state, current_parent, current_sub);
        }
    }

    _post_validation_checks();

    // 打印所有错误和警告
    for (const auto& err : errors) {
        std::cerr << err << std::endl;
    }
    for (const auto& warn : warnings) {
        std::cout << warn << std::endl;
    }

    return errors.empty();
}

void BillValidator::_process_line(const std::string& line, int line_num, State& current_state, std::string& current_parent, std::string& current_sub) {
    switch (current_state) {
        case State::EXPECT_PARENT:
            _handle_parent_state(line, line_num, current_state, current_parent);
            break;
        case State::EXPECT_SUB:
             // 在期待子标题时，如果上一个父标题已经有子标题了，需要检查上一个子标题是否有内容行
            if (!current_sub.empty() && bill_structure.count(current_parent) && bill_structure[current_parent].count(current_sub) && bill_structure[current_parent][current_sub] == 0) {
                 warnings.push_back("警告 (行 " + std::to_string(line_num) + "): 子标题 '" + current_sub + "' 缺少内容行。");
            }
            current_sub.clear(); // 为新的子标题重置
            _handle_sub_state(line, line_num, current_state, current_parent, current_sub);
            break;
        case State::EXPECT_CONTENT:
            _handle_content_state(line, line_num, current_state, current_parent, current_sub);
            break;
    }
}

// --- 文件结构验证 ---

bool BillValidator::_validate_date_and_remark(std::ifstream& file, int& line_num) {
    std::string line;
    
    // 1. 验证 DATE
    if (std::getline(file, line)) {
        line_num++;
        std::regex date_regex(R"(^DATE:\d{6}$)");
        if (!std::regex_match(line, date_regex)) {
            errors.push_back("错误 (行 " + std::to_string(line_num) + "): 文件第一行必须是 'DATE:YYYYMM' 格式。找到: '" + line + "'");
            return false;
        }
    } else {
        errors.push_back("错误: 文件为空或少于两行。");
        return false;
    }

    // 2. 验证 REMARK
    if (std::getline(file, line)) {
        line_num++;
        std::regex remark_regex(R"(^REMARK:.*)");
        if (!std::regex_match(line, remark_regex)) {
            errors.push_back("错误 (行 " + std::to_string(line_num) + "): 文件第二行必须以 'REMARK:' 开头。找到: '" + line + "'");
            return false;
        }
    } else {
        errors.push_back("错误: 文件少于两行。");
        return false;
    }
    
    return true;
}

// --- 状态机处理函数 ---

void BillValidator::_handle_parent_state(const std::string& line, int line_num, State& current_state, std::string& current_parent) {
    if (all_parent_titles.count(line)) {
        current_parent = line;
        bill_structure[current_parent]; // 注册父标题
        current_state = State::EXPECT_SUB;
    } else {
        // 检查是否看起来像一个标题，以提供更具体的错误信息
        std::regex parent_format_regex(R"([A-Z]+[\u4e00-\u9fff]+[\d]*)");
        if (std::regex_match(line, parent_format_regex)) {
             errors.push_back("错误 (行 " + std::to_string(line_num) + "): 父标题 '" + line + "' 不在配置文件中。");
        } else {
             errors.push_back("错误 (行 " + std::to_string(line_num) + "): 期望一个在配置文件中定义的父级标题, 但找到不匹配的内容: '" + line + "'");
        }
    }
}

void BillValidator::_handle_sub_state(const std::string& line, int line_num, State& current_state, const std::string& current_parent, std::string& current_sub) {
    // 检查是否是另一个父标题
    if (all_parent_titles.count(line)) {
        errors.push_back("错误 (行 " + std::to_string(line_num) + "): 父级标题 '" + current_parent + "' 缺少子标题。");
        _handle_parent_state(line, line_num, current_state, const_cast<std::string&>(current_parent)); // 重新处理为父标题
        return;
    }

    if (validation_map.count(current_parent) && validation_map.at(current_parent).count(line)) {
        current_sub = line;
        bill_structure[current_parent][current_sub] = 0; // 注册子标题，内容行为0
        current_state = State::EXPECT_CONTENT;
    } else {
        errors.push_back("错误 (行 " + std::to_string(line_num) + "): 子标题 '" + line + "' 对于父级标题 '" + current_parent + "' 无效。");
        // 保持 EXPECT_SUB 状态，等待一个有效的子标题
    }
}

void BillValidator::_handle_content_state(const std::string& line, int line_num, State& current_state, std::string& current_parent, std::string& current_sub) {
    // 检查是否是一个新的父标题
    if (all_parent_titles.count(line)) {
        // 在切换到新父标题之前，检查上一个子标题是否有内容
        if (!current_sub.empty() && bill_structure[current_parent][current_sub] == 0) {
            warnings.push_back("警告 (行 " + std::to_string(line_num) + "): 子标题 '" + current_sub + "' 缺少内容行。");
        }
        _handle_parent_state(line, line_num, current_state, current_parent);
        current_sub.clear();
        return;
    }

    // 检查是否是同一个父标题下的另一个子标题
    if (validation_map.count(current_parent) && validation_map.at(current_parent).count(line)) {
        // 在切换到新子标题之前，检查上一个子标题是否有内容
        if (!current_sub.empty() && bill_structure[current_parent][current_sub] == 0) {
            warnings.push_back("警告 (行 " + std::to_string(line_num) + "): 子标题 '" + current_sub + "' 缺少内容行。");
        }
        _handle_sub_state(line, line_num, current_state, current_parent, current_sub);
        return;
    }

    // 验证是否是内容行
    // Regex: 数字开头(可带小数), 后跟至少一个非数字非空格字符, 之后是任意数字/汉字/字母/_/-
    // 注意: C++ std::regex 对 Unicode (如汉字) 的支持可能需要特定设置，但在主流编译器 (GCC, Clang) 上通常可以工作
    std::regex content_regex(R"(^\d+(?:\.\d+)?(?:[^\d\s].*)$)");
    if (std::regex_match(line, content_regex)) {
        bill_structure[current_parent][current_sub]++;
        // 状态保持为 EXPECT_CONTENT，因为可以有多个内容行
    } else {
        errors.push_back("错误 (行 " + std::to_string(line_num) + "): 期望内容行、配置文件中有效的子标题或父标题, 但找到其他内容: '" + line + "'");
    }
}

// --- 文件结束后的最终检查 ---
void BillValidator::_post_validation_checks() {
    // 检查是否有父标题没有任何子标题被记录
    for (const auto& parent_pair : bill_structure) {
        const std::string& parent_title = parent_pair.first;
        const auto& sub_map = parent_pair.second;

        if (sub_map.empty()) {
            // 这个错误在 _handle_sub_state 中已经被更即时地捕捉了，但作为双重保障保留
            bool already_reported = false;
            for(const auto& err : errors) {
                if (err.find("父级标题 '" + parent_title + "' 缺少子标题") != std::string::npos) {
                    already_reported = true;
                    break;
                }
            }
            if (!already_reported) {
                errors.push_back("错误 (文件结尾): 父级标题 '" + parent_title + "' 缺少子标题。");
            }
        } else {
             // 检查一个父标题下的所有子标题是否都为空
            bool all_subs_empty = true;
            for (const auto& sub_pair : sub_map) {
                if (sub_pair.second > 0) {
                    all_subs_empty = false;
                    break;
                }
            }
            if (all_subs_empty) {
                warnings.push_back("警告 (文件结尾): 父标题 '" + parent_title + "' 的所有子标题均缺少内容行。");
            }
        }
    }
}
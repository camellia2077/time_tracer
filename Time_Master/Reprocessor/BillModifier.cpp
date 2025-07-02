#include "BillModifier.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <iomanip>
#include <iostream>

// --- 构造函数和主方法 ---

BillModifier::BillModifier(const nlohmann::json& config_json) {
    // 解析 modification_flags
    if (config_json.contains("modification_flags")) {
        const auto& flags = config_json["modification_flags"];
        m_config.flags.enable_summing = flags.value("enable_summing", false);
        m_config.flags.enable_autorenewal = flags.value("enable_autorenewal", false);
        m_config.flags.enable_cleanup = flags.value("enable_cleanup", false);
        m_config.flags.enable_sorting = flags.value("enable_sorting", false);
        m_config.flags.preserve_metadata_lines = flags.value("preserve_metadata_lines", false);
    }

    // 解析 formatting_rules
    if (config_json.contains("formatting_rules")) {
        const auto& formatting = config_json["formatting_rules"];
        m_config.formatting.lines_after_parent_section = formatting.value("lines_after_parent_section", 1);
        m_config.formatting.lines_after_parent_title = formatting.value("lines_after_parent_title", 1);
        m_config.formatting.lines_between_sub_items = formatting.value("lines_between_sub_items", 1);
    }

    // 解析 auto_renewal_rules
    if (config_json.contains("auto_renewal_rules")) {
        const auto& renewal_rules = config_json["auto_renewal_rules"];
        for (auto it = renewal_rules.begin(); it != renewal_rules.end(); ++it) {
            const std::string& category = it.key();
            const nlohmann::json& items = it.value();
            for (const auto& item_json : items) {
                m_config.auto_renewal_rules[category].push_back({
                    item_json.value("amount", 0.0),
                    item_json.value("description", "")
                });
            }
        }
    }

    // 新增：解析 metadata_prefixes
    if (config_json.contains("metadata_prefixes") && config_json["metadata_prefixes"].is_array()) {
        for (const auto& prefix_json : config_json["metadata_prefixes"]) {
            if (prefix_json.is_string()) {
                m_config.metadata_prefixes.push_back(prefix_json.get<std::string>());
            }
        }
    }
}

std::string BillModifier::modify(const std::string& bill_content) {
    std::vector<std::string> lines = _split_string_by_lines(bill_content);
    
    _perform_initial_modifications(lines);
    
    return _process_structured_modifications(lines);
}


// --- 阶段 1: 初始修改实现 ---

void BillModifier::_perform_initial_modifications(std::vector<std::string>& lines) {
    if (m_config.flags.enable_summing) {
        for (std::string& line : lines) {
            _sum_up_line(line);
        }
    }

    if (m_config.flags.enable_autorenewal) {
        for (const auto& pair : m_config.auto_renewal_rules) {
            const std::string& category_title = pair.first;
            const auto& items_to_add = pair.second;

            auto category_it = std::find(lines.begin(), lines.end(), category_title);
            if (category_it == lines.end()) {
                continue;
            }

            auto content_start_it = category_it + 1;
            auto content_end_it = content_start_it;
            while (content_end_it != lines.end() && !_is_title(*content_end_it) && !content_end_it->empty()) {
                ++content_end_it;
            }

            for (const auto& item_to_add : items_to_add) {
                bool found = false;
                for (auto it = content_start_it; it != content_end_it; ++it) {
                    if (it->find(item_to_add.description) != std::string::npos) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(2) << item_to_add.amount
                       << " " << item_to_add.description << "(auto-renewal)";
                    lines.insert(content_end_it, ss.str());
                    
                     content_end_it = content_start_it;
                     while (content_end_it != lines.end() && !_is_title(*content_end_it) && !content_end_it->empty()) {
                         ++content_end_it;
                     }
                }
            }
        }
    }
}


void BillModifier::_sum_up_line(std::string& line) {
    std::regex expr_regex(R"(^([\d\.\s]+\+[\d\.\s\+]+)(.*))");
    std::smatch match;

    if (std::regex_search(line, match, expr_regex) && match.size() > 1) {
        std::string expression = match[1].str();
        std::string description = match[2].str();
        
        expression.erase(std::remove(expression.begin(), expression.end(), ' '), expression.end());
        std::replace(expression.begin(), expression.end(), '+', ' ');

        std::stringstream ss(expression);
        double sum = 0.0, num;
        while (ss >> num) {
            sum += num;
        }

        std::stringstream result_ss;
        result_ss << std::fixed << std::setprecision(2) << sum << description;
        line = result_ss.str();
    }
}


// --- 阶段 2: 结构化修改实现 ---

std::string BillModifier::_process_structured_modifications(const std::vector<std::string>& lines) {
    std::vector<std::string> metadata_lines;
    std::vector<ParentItem> bill_structure = _parse_into_structure(lines, metadata_lines);

    if (m_config.flags.enable_sorting) {
        _sort_bill_structure(bill_structure);
    }

    if (m_config.flags.enable_cleanup) {
        _cleanup_bill_structure(bill_structure);
    }

    return _reconstruct_content_with_formatting(bill_structure, metadata_lines);
}


std::vector<ParentItem> BillModifier::_parse_into_structure(const std::vector<std::string>& lines, std::vector<std::string>& metadata_lines) const {
    std::vector<ParentItem> structure;
    ParentItem* current_parent = nullptr;
    SubItem* current_sub_item = nullptr;

    std::vector<std::string> temp_lines;
    for(const auto& line : lines) {
        if (m_config.flags.preserve_metadata_lines && _is_metadata_line(line)) {
            metadata_lines.push_back(line);
            continue;
        }
        
        std::string temp = line;
        if(!_trim(temp).empty()){
            temp_lines.push_back(temp);
        }
    }

    for (size_t i = 0; i < temp_lines.size(); ++i) {
        const std::string& line = temp_lines[i];

        if (_is_title(line)) {
            bool is_parent = false;
            if (i + 1 < temp_lines.size()) {
                if (_is_title(temp_lines[i+1])) {
                    is_parent = true;
                }
            }

            if (is_parent) {
                structure.emplace_back();
                current_parent = &structure.back();
                current_parent->title = line;
                current_sub_item = nullptr;
            } else {
                if (!current_parent) {
                    structure.emplace_back();
                    current_parent = &structure.back();
                    current_parent->title = "Default Parent";
                }
                current_parent->sub_items.emplace_back();
                current_sub_item = &current_parent->sub_items.back();
                current_sub_item->title = line;
            }
        } else {
            if (current_sub_item) {
                current_sub_item->contents.push_back(line);
            }
        }
    }
    return structure;
}

void BillModifier::_sort_bill_structure(std::vector<ParentItem>& bill_structure) const {
    for (auto& parent : bill_structure) {
        for (auto& sub_item : parent.sub_items) {
            std::sort(sub_item.contents.begin(), sub_item.contents.end(), 
                [](const std::string& a, const std::string& b) {
                    double val_a = _get_numeric_value_from_content(a);
                    double val_b = _get_numeric_value_from_content(b);
                    if (val_a != val_b) {
                        return val_a > val_b;
                    }
                    return a < b;
                });
        }
    }
}

void BillModifier::_cleanup_bill_structure(std::vector<ParentItem>& bill_structure) const {
    for (auto& parent : bill_structure) {
        parent.sub_items.erase(
            std::remove_if(parent.sub_items.begin(), parent.sub_items.end(),
                [](const SubItem& sub) {
                    return sub.contents.empty();
                }),
            parent.sub_items.end());
    }

    bill_structure.erase(
        std::remove_if(bill_structure.begin(), bill_structure.end(),
            [](const ParentItem& parent) {
                return parent.sub_items.empty();
            }),
        bill_structure.end());
}


std::string BillModifier::_reconstruct_content_with_formatting(const std::vector<ParentItem>& bill_structure, const std::vector<std::string>& metadata_lines) const {
    std::stringstream ss;
    
    for (const auto& meta_line : metadata_lines) {
        ss << meta_line << "\n";
    }
    
    if (!metadata_lines.empty() && !bill_structure.empty()) {
        ss << "\n";
    }

    bool first_parent = true;
    for (const auto& parent : bill_structure) {
        if (!first_parent) {
            for(int i = 0; i < m_config.formatting.lines_after_parent_section; ++i) ss << "\n";
        }
        ss << parent.title;
        first_parent = false;

        for(int i = 0; i < m_config.formatting.lines_after_parent_title; ++i) ss << "\n";

        bool first_sub_item = true;
        for (const auto& sub_item : parent.sub_items) {
            if (!first_sub_item) {
                for(int i = 0; i < m_config.formatting.lines_between_sub_items; ++i) ss << "\n";
            }
            ss << sub_item.title;
            first_sub_item = false;

            for (const auto& content : sub_item.contents) {
                ss << "\n" << content;
            }
        }
    }
    return ss.str();
}


// --- 辅助函数实现 ---

// 修改：重写函数以使用 m_config 中的前缀列表
bool BillModifier::_is_metadata_line(const std::string& line) const {
    // 遍历从 JSON 配置中读取的所有前缀
    for (const auto& prefix : m_config.metadata_prefixes) {
        // 检查行是否以其中任何一个前缀开头 (rfind a at pos 0)
        if (line.rfind(prefix, 0) == 0) {
            return true;
        }
    }
    return false;
}

double BillModifier::_get_numeric_value_from_content(const std::string& content_line) {
    try {
        size_t pos;
        double val = std::stod(content_line, &pos);
        if (pos == 0) return 0.0;
        return val;
    } catch (const std::invalid_argument&) {
        return 0.0;
    }
}

bool BillModifier::_is_title(const std::string& line) {
    if (line.empty()) return false;
    for(char c : line){
        if(!isspace(c)){
            return !isdigit(c);
        }
    }
    return false;
}

std::vector<std::string> BillModifier::_split_string_by_lines(const std::string& str) {
    std::vector<std::string> lines;
    std::string line;
    std::istringstream stream(str);
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string& BillModifier::_trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}
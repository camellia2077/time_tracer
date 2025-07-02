#ifndef BILL_MODIFIER_H
#define BILL_MODIFIER_H

#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp" // 需要 nlohmann/json 库

// 用于存储从 JSON 解析的配置规则
struct Config {
    struct ModificationFlags {
        bool enable_summing = false;
        bool enable_autorenewal = false;
        bool enable_cleanup = false;
        bool enable_sorting = false;
        bool preserve_metadata_lines = false;
    } flags;

    struct FormattingRules {
        int lines_after_parent_section = 1;
        int lines_after_parent_title = 1;
        int lines_between_sub_items = 0;
    } formatting;

    struct AutoRenewalItem {
        double amount;
        std::string description;
    };
    std::map<std::string, std::vector<AutoRenewalItem>> auto_renewal_rules;

    // 新增：用于存储元数据行前缀的列表
    std::vector<std::string> metadata_prefixes;
};

// 用于表示账单的层级结构
using ContentItem = std::string;

struct SubItem {
    std::string title;
    std::vector<ContentItem> contents;
};

struct ParentItem {
    std::string title;
    std::vector<SubItem> sub_items;
};


/**
 * @class BillModifier
 * @brief 根据 JSON 配置修改账单文件内容。
 *
 * 该类实现了对账单文本的计算、添加、排序、清理和格式化功能。
 * 它首先对文件行进行初始修改（求和、自动续费），然后将文件解析为
 * 结构化数据，执行排序和清理，最后根据格式化规则重新生成内容。
 */
class BillModifier {
public:
    /**
     * @brief 构造函数，使用提供的 JSON 对象初始化修改器。
     * @param config_json nlohmann::json 对象，包含所有配置规则。
     */
    explicit BillModifier(const nlohmann::json& config_json);

    /**
     * @brief 对提供的账单内容执行所有已启用的修改。
     * @param bill_content 作为单个字符串的原始账单内容。
     * @return 经过所有修改和格式化后的新账单内容。
     */
    std::string modify(const std::string& bill_content);

private:
    Config m_config; // 存储解析后的配置

    // 阶段 1: 直接修改文件行
    void _perform_initial_modifications(std::vector<std::string>& lines);
    void _sum_up_line(std::string& line);

    // 阶段 2: 结构化修改
    std::string _process_structured_modifications(const std::vector<std::string>& lines);
    
    // -- 解析 --
    std::vector<ParentItem> _parse_into_structure(const std::vector<std::string>& lines, std::vector<std::string>& metadata_lines) const;

    // -- 修改 --
    void _sort_bill_structure(std::vector<ParentItem>& bill_structure) const;
    void _cleanup_bill_structure(std::vector<ParentItem>& bill_structure) const;

    // -- 重建 --
    std::string _reconstruct_content_with_formatting(const std::vector<ParentItem>& bill_structure, const std::vector<std::string>& metadata_lines) const;

    // -- 辅助函数 --
    // 修改：移除了 static，添加了 const
    bool _is_metadata_line(const std::string& line) const;
    static double _get_numeric_value_from_content(const std::string& content_line);
    static bool _is_title(const std::string& line);
    static std::vector<std::string> _split_string_by_lines(const std::string& str);
    static std::string& _trim(std::string& s);
};

#endif // BILL_MODIFIER_H
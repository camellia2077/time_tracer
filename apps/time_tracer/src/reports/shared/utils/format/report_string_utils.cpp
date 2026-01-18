// reports/shared/utils/format/report_string_utils.cpp
#include "report_string_utils.hpp"

std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string format_multiline_for_list(const std::string& text, int indent_spaces, const std::string& line_suffix) {
    if (text.empty()) return "";
    
    std::string indent(indent_spaces, ' ');
    // 将 "\n" 替换为 "后缀 + \n + 缩进"
    std::string replacement = line_suffix + "\n" + indent;
    
    return replace_all(text, "\n", replacement);
}
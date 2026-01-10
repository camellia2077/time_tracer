// cli/CliParser.cpp
#include "CliParser.hpp"
#include <stdexcept>
#include <algorithm>
#include <sstream> // [新增] 用于字符串分割

CliParser::CliParser(const std::vector<std::string>& args) : raw_args_(args) {
    if (args.size() < 2) {
        throw std::runtime_error("No command provided.");
    }
    command_ = raw_args_[1];
    parse();
}

std::string CliParser::get_command() const {
    return command_;
}

const std::vector<std::string>& CliParser::get_filtered_args() const {
    return filtered_args_;
}

std::string CliParser::get_raw_arg(size_t index) const {
    if (index >= raw_args_.size()) {
        throw std::out_of_range("Argument index out of range.");
    }
    return raw_args_[index];
}

void CliParser::parse() {
    filtered_args_ = filter_global_options(raw_args_);
}

std::vector<std::string> CliParser::filter_global_options(const std::vector<std::string>& original_args) {
    std::vector<std::string> filtered_args;
    for (size_t i = 0; i < original_args.size(); ++i) {
        const auto& arg = original_args[i];
        // [修改] 增加 --save-processed 到过滤列表
        if (arg == "-o" || arg == "--output" || arg == "-f" || arg == "--format" || arg == "--date-check") {
            if (i + 1 < original_args.size()) {
                i++; 
            }
            continue;
        }
        // [新增] 单独处理不需要参数的 flag
        if (arg == "--save-processed") {
            continue;
        }
        filtered_args.push_back(arg);
    }
    return filtered_args;
}
bool CliParser::should_save_processed() const {
    return std::find(raw_args_.begin(), raw_args_.end(), "--save-processed") != raw_args_.end();
}

std::optional<std::string> CliParser::get_output_path() const {
    auto it_o = std::find(raw_args_.begin(), raw_args_.end(), "-o");
    if (it_o != raw_args_.end() && std::next(it_o) != raw_args_.end()) {
        return *std::next(it_o);
    }

    auto it_output = std::find(raw_args_.begin(), raw_args_.end(), "--output");
    if (it_output != raw_args_.end() && std::next(it_output) != raw_args_.end()) {
        return *std::next(it_output);
    }
    
    return std::nullopt;
}

// [核心修改] 将原 get_report_format 替换为支持多格式的 get_report_formats
std::vector<ReportFormat> CliParser::get_report_formats() const {
    std::string format_str;
    auto it_f = std::find(raw_args_.begin(), raw_args_.end(), "-f");
    if (it_f != raw_args_.end() && std::next(it_f) != raw_args_.end()) {
        format_str = *std::next(it_f);
    } else {
        auto it_format = std::find(raw_args_.begin(), raw_args_.end(), "--format");
        if (it_format != raw_args_.end() && std::next(it_format) != raw_args_.end()) {
            format_str = *std::next(it_format);
        } else {
            // 默认只返回 Markdown
            return {ReportFormat::Markdown}; 
        }
    }

    std::vector<ReportFormat> formats;
    std::stringstream ss(format_str);
    std::string segment;

    while (std::getline(ss, segment, ',')) {
        // 1. 去除首尾空白字符
        size_t first = segment.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) continue; // 跳过空片段
        
        size_t last = segment.find_last_not_of(" \t\n\r");
        std::string trimmed = segment.substr(first, (last - first + 1));

        // 2. 解析格式
        if (trimmed == "md" || trimmed == "markdown") {
            formats.push_back(ReportFormat::Markdown);
        } else if (trimmed == "tex" || trimmed == "latex") {
            formats.push_back(ReportFormat::LaTeX);
        } else if (trimmed == "typ" || trimmed == "typst") {
            formats.push_back(ReportFormat::Typ);
        } else {
            throw std::runtime_error("Unsupported format specified: '" + trimmed + "'. Supported formats: md, tex, typ.");
        }
    }

    if (formats.empty()) {
        return {ReportFormat::Markdown};
    }

    return formats;
}

// [核心修改] 实现获取日期检查模式的逻辑
DateCheckMode CliParser::get_date_check_mode() const {
    auto it = std::find(raw_args_.begin(), raw_args_.end(), "--date-check");
    if (it != raw_args_.end() && std::next(it) != raw_args_.end()) {
        std::string mode_str = *std::next(it);
        if (mode_str == "continuity") {
            return DateCheckMode::Continuity;
        } else if (mode_str == "full" || mode_str == "strict") {
            return DateCheckMode::Full;
        } else {
             throw std::runtime_error("Invalid argument for --date-check: '" + mode_str + "'. Expected 'continuity' or 'full'.");
        }
    }
    // 默认不开启检查
    return DateCheckMode::None;
}
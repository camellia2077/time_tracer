// cli/CliParser.cpp
#include "CliParser.hpp"
#include <stdexcept>
#include <algorithm>
#include <sstream>

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
        
        // 过滤带参数的选项
        if (arg == "-o" || arg == "--output" || arg == "-f" || arg == "--format" || arg == "--date-check") {
            if (i + 1 < original_args.size()) {
                i++; 
            }
            continue;
        }
        
        // [修改] 过滤布尔开关选项：新增 --no-date-check
        if (arg == "--save-processed" || arg == "--no-save" || arg == "--no-date-check") {
            continue;
        }
        
        filtered_args.push_back(arg);
    }
    return filtered_args;
}

bool CliParser::should_save_processed() const {
    return std::find(raw_args_.begin(), raw_args_.end(), "--save-processed") != raw_args_.end();
}

bool CliParser::should_skip_save() const {
    return std::find(raw_args_.begin(), raw_args_.end(), "--no-save") != raw_args_.end();
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
            return {ReportFormat::Markdown}; 
        }
    }

    std::vector<ReportFormat> formats;
    std::stringstream ss(format_str);
    std::string segment;

    while (std::getline(ss, segment, ',')) {
        size_t first = segment.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) continue;
        
        size_t last = segment.find_last_not_of(" \t\n\r");
        std::string trimmed = segment.substr(first, (last - first + 1));

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

// [修改] 实现逻辑以支持 optional 和 --no-date-check
std::optional<DateCheckMode> CliParser::get_date_check_mode() const {
    // 1. 检查 --date-check 参数
    auto it = std::find(raw_args_.begin(), raw_args_.end(), "--date-check");
    if (it != raw_args_.end() && std::next(it) != raw_args_.end()) {
        std::string mode_str = *std::next(it);
        if (mode_str == "continuity") {
            return DateCheckMode::Continuity;
        } else if (mode_str == "full" || mode_str == "strict") {
            return DateCheckMode::Full;
        } else if (mode_str == "none" || mode_str == "off") {
            return DateCheckMode::None;
        } else {
             throw std::runtime_error("Invalid argument for --date-check: '" + mode_str + "'. Expected 'continuity', 'full', or 'none'.");
        }
    }

    // 2. 检查 --no-date-check 参数
    auto it_no = std::find(raw_args_.begin(), raw_args_.end(), "--no-date-check");
    if (it_no != raw_args_.end()) {
        return DateCheckMode::None;
    }

    // 3. 未指定
    return std::nullopt;
}
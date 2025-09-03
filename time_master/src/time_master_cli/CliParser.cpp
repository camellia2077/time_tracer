// time_master_cli/CliParser.cpp
#include "CliParser.hpp"
#include <stdexcept>
#include <algorithm>

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
    // 这个辅助函数现在是 CliParser 的一部分
    filtered_args_ = filter_global_options(raw_args_);
}

// 核心修改：所有与参数解析相关的函数都移动到这里
std::vector<std::string> CliParser::filter_global_options(const std::vector<std::string>& original_args) {
    std::vector<std::string> filtered_args;
    for (size_t i = 0; i < original_args.size(); ++i) {
        const auto& arg = original_args[i];
        if (arg == "-o" || arg == "--output" || arg == "-f" || arg == "--format") {
            if (i + 1 < original_args.size()) {
                i++; // 跳过选项的值
            }
            continue;
        }
        filtered_args.push_back(arg);
    }
    return filtered_args;
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

ReportFormat CliParser::get_report_format() const {
    std::string format_str;
    auto it_f = std::find(raw_args_.begin(), raw_args_.end(), "-f");
    if (it_f != raw_args_.end() && std::next(it_f) != raw_args_.end()) {
        format_str = *std::next(it_f);
    } else {
        auto it_format = std::find(raw_args_.begin(), raw_args_.end(), "--format");
        if (it_format != raw_args_.end() && std::next(it_format) != raw_args_.end()) {
            format_str = *std::next(it_format);
        } else {
            return ReportFormat::Markdown; // 默认值
        }
    }

    if (format_str == "md" || format_str == "markdown") return ReportFormat::Markdown;
    if (format_str == "tex") return ReportFormat::LaTeX;
    if (format_str == "typ") return ReportFormat::Typ;
     
    throw std::runtime_error("Unsupported format specified: '" + format_str + "'. Supported formats: md, tex, typ.");
}
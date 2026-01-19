// cli/framework/core/command_parser.cpp
#include "cli/framework/core/command_parser.hpp"
#include <stdexcept>
#include <algorithm>
#include <iterator>

// [修改] 初始化配置
CommandParser::CommandParser(const std::vector<std::string>& args, const ParserConfig& config) 
    : raw_args_(args), config_(config) {
    if (args.size() < 2) {
        throw std::runtime_error("No command provided.");
    }
    command_ = raw_args_[1];
    parse();
}

std::string CommandParser::get_command() const {
    return command_;
}

const std::vector<std::string>& CommandParser::get_filtered_args() const {
    return filtered_args_;
}

std::string CommandParser::get_raw_arg(size_t index) const {
    if (index >= raw_args_.size()) {
        throw std::out_of_range("Argument index out of range.");
    }
    return raw_args_[index];
}

std::optional<std::string> CommandParser::get_option(const std::vector<std::string>& keys) const {
    for (const auto& key : keys) {
        auto it = std::find(raw_args_.begin(), raw_args_.end(), key);
        if (it != raw_args_.end() && std::next(it) != raw_args_.end()) {
            return *std::next(it);
        }
    }
    return std::nullopt;
}

bool CommandParser::has_flag(const std::vector<std::string>& keys) const {
    for (const auto& key : keys) {
        if (std::find(raw_args_.begin(), raw_args_.end(), key) != raw_args_.end()) {
            return true;
        }
    }
    return false;
}

void CommandParser::parse() {
    filtered_args_ = filter_global_options(raw_args_);
}

// [修改] 使用 config_ 中的定义进行过滤，彻底移除硬编码
std::vector<std::string> CommandParser::filter_global_options(const std::vector<std::string>& original_args) {
    std::vector<std::string> filtered;
    
    for (size_t i = 0; i < original_args.size(); ++i) {
        const auto& arg = original_args[i];
        
        // 1. 检查是否是带值选项 (Value Options)
        if (std::find(config_.global_value_options.begin(), config_.global_value_options.end(), arg) 
            != config_.global_value_options.end()) {
            i++; // 跳过 key 和 value
            continue;
        }
        
        // 2. 检查是否是布尔选项 (Flags)
        if (std::find(config_.global_flag_options.begin(), config_.global_flag_options.end(), arg) 
            != config_.global_flag_options.end()) {
            continue; // 仅跳过 key
        }
        
        filtered.push_back(arg);
    }
    return filtered;
}
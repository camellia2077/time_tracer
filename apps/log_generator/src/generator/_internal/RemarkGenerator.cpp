// generator/_internal/RemarkGenerator.cpp
#include "RemarkGenerator.h"
#include <format>

RemarkGenerator::RemarkGenerator(const std::optional<DailyRemarkConfig>& config, std::mt19937& gen)
    : remark_config_(config), gen_(gen) {
    if (remark_config_ && !remark_config_->contents.empty()) {
        selector_ = std::make_unique<std::uniform_int_distribution<>>(0, static_cast<int>(remark_config_->contents.size()) - 1);
        should_generate_ = std::make_unique<std::bernoulli_distribution>(remark_config_->generation_chance);
    }
}

std::optional<std::string> RemarkGenerator::try_generate() {
    if (should_generate_ && (*should_generate_)(gen_)) {
        const std::string& content = remark_config_->contents[(*selector_)(gen_)];
        return std::format("{}{}", remark_config_->prefix, content);
    }
    return std::nullopt;
}
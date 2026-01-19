// cli/framework/core/command_validator.hpp
#ifndef CLI_FRAMEWORK_CORE_COMMAND_VALIDATOR_HPP_
#define CLI_FRAMEWORK_CORE_COMMAND_VALIDATOR_HPP_

#include "cli/framework/core/command_parser.hpp" // [修复] 必须包含
#include "cli/framework/core/arg_definitions.hpp"
#include <vector>
#include <stdexcept>

class CommandValidator {
public:
    static ParsedArgs validate(const CommandParser& parser, const std::vector<ArgDef>& defs) {
        ParsedArgs result;
        auto positional_args = parser.get_filtered_args(); 
        size_t base_index = 2; 

        for (const auto& def : defs) {
            if (def.type == ArgType::Positional) {
                size_t real_index = base_index + def.position_index;
                if (real_index < positional_args.size()) {
                    result.values_[def.name] = positional_args[real_index];
                } else if (def.required) {
                    throw std::runtime_error("Missing required argument: <" + def.name + ">");
                }
            } 
            else if (def.type == ArgType::Option) {
                auto val = parser.get_option(def.keys);
                if (val) {
                    result.values_[def.name] = *val;
                } else if (def.required) {
                    throw std::runtime_error("Missing required option: " + def.keys[0]);
                } else if (!def.default_value.empty()) {
                    result.values_[def.name] = def.default_value;
                }
            }
            else if (def.type == ArgType::Flag) {
                if (parser.has_flag(def.keys)) {
                    result.values_[def.name] = "true";
                }
            }
        }
        return result;
    }
};

#endif
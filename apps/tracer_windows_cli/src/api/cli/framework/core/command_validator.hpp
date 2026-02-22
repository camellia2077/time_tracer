// api/cli/framework/core/command_validator.hpp
#ifndef API_CLI_FRAMEWORK_CORE_COMMAND_VALIDATOR_H_
#define API_CLI_FRAMEWORK_CORE_COMMAND_VALIDATOR_H_

#include <stdexcept>
#include <vector>

#include "api/cli/framework/core/arg_definitions.hpp"
#include "api/cli/framework/core/command_parser.hpp" // [修复] 必须包含

class CommandValidator {
public:
  static auto Validate(const CommandParser &parser,
                       const std::vector<ArgDef> &defs) -> ParsedArgs {
    constexpr size_t kAppArgsOffset = 2;
    ParsedArgs result;
    auto positional_args = parser.GetFilteredArgs();
    size_t base_index = kAppArgsOffset;

    for (const auto &def : defs) {
      if (def.type == ArgType::kPositional) {
        size_t real_index = base_index + def.position_index;
        if (real_index < positional_args.size()) {
          result.values_[def.name] = positional_args[real_index];
        } else if (def.required) {
          throw std::runtime_error("Missing required argument: <" + def.name +
                                   ">");
        }
      } else if (def.type == ArgType::kOption) {
        auto val = parser.GetOption(def.keys);
        if (val) {
          result.values_[def.name] = *val;
        } else if (def.required) {
          throw std::runtime_error("Missing required option: " + def.keys[0]);
        } else if (!def.default_value.empty()) {
          result.values_[def.name] = def.default_value;
        }
      } else if (def.type == ArgType::kFlag) {
        if (parser.HasFlag(def.keys)) {
          result.values_[def.name] = "true";
        }
      }
    }
    return result;
  }
};

#endif

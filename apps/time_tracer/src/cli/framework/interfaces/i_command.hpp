// cli/framework/interfaces/i_command.hpp
#ifndef CLI_FRAMEWORK_INTERFACES_I_COMMAND_HPP_
#define CLI_FRAMEWORK_INTERFACES_I_COMMAND_HPP_

#include "cli/framework/core/command_parser.hpp"

#include <string>

class ICommand {
public:
    virtual ~ICommand() = default;
    
    // Every command MUST have an execute method
    virtual void execute(const CommandParser& parser) = 0;
    
    // This makes implementing it in subclasses optional.
    virtual std::string get_help() const { return ""; }
};

#endif // CLI_FRAMEWORK_INTERFACES_I_COMMAND_HPP_
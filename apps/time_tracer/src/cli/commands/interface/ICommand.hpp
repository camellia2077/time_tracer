// cli/commands/interface/ICommand.hpp
#ifndef I_COMMAND_HPP
#define I_COMMAND_HPP

#include "cli/CliParser.hpp"
#include <string>

class ICommand {
public:
    virtual ~ICommand() = default;
    
    // Every command MUST have an execute method
    virtual void execute(const CliParser& parser) = 0;
    
    // [FIX] Changed from pure virtual to a regular virtual function with a default implementation.
    // This makes implementing it in subclasses optional.
    virtual std::string get_help() const { return ""; }
};

#endif // I_COMMAND_HPP
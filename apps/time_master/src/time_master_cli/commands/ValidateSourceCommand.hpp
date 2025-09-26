// time_master_cli/commands/ValidateSourceCommand.hpp
#ifndef VALIDATE_SOURCE_COMMAND_HPP
#define VALIDATE_SOURCE_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class ValidateSourceCommand : public ICommand {
public:
    explicit ValidateSourceCommand(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // VALIDATE_SOURCE_COMMAND_HPP
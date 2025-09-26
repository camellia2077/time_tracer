// time_master_cli/commands/ValidateOutputCommand.hpp
#ifndef VALIDATE_OUTPUT_COMMAND_HPP
#define VALIDATE_OUTPUT_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class ValidateOutputCommand : public ICommand {
public:
    explicit ValidateOutputCommand(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // VALIDATE_OUTPUT_COMMAND_HPP
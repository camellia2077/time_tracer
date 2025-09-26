// time_master_cli/commands/ConvertCommand.hpp
#ifndef CONVERT_COMMAND_HPP
#define CONVERT_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class ConvertCommand : public ICommand {
public:
    explicit ConvertCommand(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // CONVERT_COMMAND_HPP
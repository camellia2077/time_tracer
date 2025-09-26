// time_master_cli/commands/pipeline/ValidateOutput.hpp
#ifndef VALIDATE_OUTPUT_HPP
#define VALIDATE_OUTPUT_HPP

#include "time_master_cli/commands/interface/ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class ValidateOutput : public ICommand {
public:
    explicit ValidateOutput(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // VALIDATE_OUTPUT_HPP
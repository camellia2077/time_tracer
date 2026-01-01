// cli/commands/pipeline/ValidateSource.hpp
#ifndef VALIDATE_SOURCE_HPP
#define VALIDATE_SOURCE_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class ValidateSource : public ICommand {
public:
    explicit ValidateSource(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // VALIDATE_SOURCE_HPP
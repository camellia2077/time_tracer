// cli/commands/pipeline/Convert.hpp
#ifndef CONVERT_HPP
#define CONVERT_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class Convert : public ICommand {
public:
    explicit Convert(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // CONVERT_HPP
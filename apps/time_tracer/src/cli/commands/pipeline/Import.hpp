// cli/commands/pipeline/Import.hpp
#ifndef IMPORT_HPP
#define IMPORT_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class Import : public ICommand {
public:
    explicit Import(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // IMPORT_HPP
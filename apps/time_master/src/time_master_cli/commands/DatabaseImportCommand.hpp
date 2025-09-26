// time_master_cli/commands/DatabaseImportCommand.hpp
#ifndef DATABASE_IMPORT_COMMAND_HPP
#define DATABASE_IMPORT_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/FileHandler.hpp"

class DatabaseImportCommand : public ICommand {
public:
    explicit DatabaseImportCommand(FileHandler& file_handler);
    void execute(const CliParser& parser) override;
private:
    FileHandler& file_handler_;
};

#endif // DATABASE_IMPORT_COMMAND_HPP
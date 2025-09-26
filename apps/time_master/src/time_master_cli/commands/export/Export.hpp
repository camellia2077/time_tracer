// time_master_cli/commands/export/Export.hpp
#ifndef EXPORT_HPP
#define EXPORT_HPP

#include "time_master_cli/commands/interface/ICommand.hpp"
#include "action_handler/ReportHandler.hpp"

class Export : public ICommand {
public:
    explicit Export(ReportHandler& report_handler);
    void execute(const CliParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // EXPORT_HPP
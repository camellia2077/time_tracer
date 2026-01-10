// cli/commands/export/Export.hpp
#ifndef EXPORT_HPP
#define EXPORT_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "core/ReportHandler.hpp"

class Export : public ICommand {
public:
    explicit Export(ReportHandler& report_handler);
    void execute(const CliParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // EXPORT_HPP
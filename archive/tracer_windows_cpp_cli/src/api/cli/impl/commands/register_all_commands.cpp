// api/cli/impl/commands/register_all_commands.cpp
#include "api/cli/impl/commands/register_all_commands.hpp"

namespace tracer_core::cli::impl::commands {

void RegisterExportCommand();
void RegisterCryptoCommand();
void RegisterChartCommand();
void RegisterDoctorCommand();
void RegisterQueryCommand();
void RegisterTreeCommand();
void RegisterConvertCommand();
void RegisterImportCommand();
void RegisterIngestCommand();
void RegisterValidateLogicCommand();
void RegisterValidateStructureCommand();

void RegisterAllCommands() {
  static const bool kRegistered = []() -> bool {
    RegisterExportCommand();
    RegisterCryptoCommand();
    RegisterChartCommand();
    RegisterDoctorCommand();
    RegisterQueryCommand();
    RegisterTreeCommand();
    RegisterConvertCommand();
    RegisterImportCommand();
    RegisterIngestCommand();
    RegisterValidateLogicCommand();
    RegisterValidateStructureCommand();
    return true;
  }();
  (void)kRegistered;
}

} // namespace tracer_core::cli::impl::commands

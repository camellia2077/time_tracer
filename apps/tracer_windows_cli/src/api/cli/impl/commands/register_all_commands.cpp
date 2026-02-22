// api/cli/impl/commands/register_all_commands.cpp
#include "api/cli/impl/commands/register_all_commands.hpp"

namespace time_tracer::cli::impl::commands {

void RegisterExportCommand();
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

} // namespace time_tracer::cli::impl::commands

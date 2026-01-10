// cli/CommandRegistry.cpp
#include "cli/CommandRegistry.hpp"

CommandRegistry& CommandRegistry::instance() {
    static CommandRegistry inst;
    return inst;
}

void CommandRegistry::register_command(const std::string& name, CommandCreator creator) {
    creators_[name] = creator;
}

std::unique_ptr<ICommand> CommandRegistry::create_command(const std::string& name, CliContext& ctx) {
    auto it = creators_.find(name);
    if (it != creators_.end()) {
        return it->second(ctx);
    }
    return nullptr;
}

std::vector<std::string> CommandRegistry::get_registered_commands() const {
    std::vector<std::string> names;
    for (const auto& pair : creators_) {
        names.push_back(pair.first);
    }
    return names;
}

CommandRegistrar::CommandRegistrar(const std::string& name, CommandCreator creator) {
    CommandRegistry::instance().register_command(name, creator);
}
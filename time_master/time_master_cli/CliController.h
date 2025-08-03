// cli/CliController.h
#ifndef CLI_CONTROLLER_H
#define CLI_CONTROLLER_H

#include <string>
#include <vector>
#include "queries/shared/ReportFormat.h"

// 前向声明
class ActionHandler;
class FileController;

/**
 * @class CliController
 * @brief Handles all Command-Line Interface (CLI) logic.
 *
 * This class parses command-line arguments and invokes the appropriate
 * ActionHandler methods based on those arguments. It serves as the main
 * business logic hub for the CLI.
 */
class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    /**
     * @brief Executes the command specified by the command-line arguments.
     *
     * This is the main entry point for the class. It will parse the command
     * and dispatch to the appropriate handler function.
     * Throws std::runtime_error on failure.
     */
    void execute();

private:
    std::vector<std::string> args_;
    std::string command_;
    ActionHandler* action_handler_;
    FileController* file_controller_;

    // --- Private helper functions for handling command branches ---
    void handle_run_all();
    void handle_preprocess();
    void handle_database_import();
    void handle_query();
    void handle_export();

    /**
     * @brief Parses the format option (-f, --format) from the command line.
     * @return A ReportFormat enum value. Defaults to Markdown if not specified.
     */
    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_H
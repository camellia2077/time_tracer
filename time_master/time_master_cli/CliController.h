// cli/CliController.h
#include "common/pch.h"
#ifndef CLI_CONTROLLER_H
#define CLI_CONTROLLER_H

#include <string>
#include <vector>
#include <functional> // Required for std::function
#include "queries/shared/ReportFormat.h"

// Forward declarations to avoid including heavy headers
class ActionHandler;
class FileController;

/**
 * @class CliController
 * @brief Handles all Command Line Interface (CLI) logic.
 *
 * This class parses command-line arguments and invokes the appropriate
 * ActionHandler methods based on those arguments. It serves as the main
 * business logic hub for the CLI.
 */
class CliController {
public:
    /**
     * @brief Constructs a CliController instance.
     * @param args The list of command-line arguments from main.
     */
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
    void handle_full_pipeline();
    void handle_manual_preprocessing();
    void handle_database_import();
    void handle_query();
    void handle_export();

    /**
     * @brief Generic helper to export a single report.
     * @param report_type The type of report (e.g., "日报", "月报").
     * @param identifier The report's identifier (e.g., "20240115", "202401").
     * @param content_generator A lambda function that returns the report content.
     * @param format The format of the report.
     */
    void export_single_report(
        const std::string& report_type,
        const std::string& identifier,
        std::function<std::string()> content_generator,
        ReportFormat format
    ) const; // <<< CORRECTION: Added 'const' to match the .cpp and added semicolon

    /**
     * @brief Parses the format option (-f, --format) from the command line.
     * @return A ReportFormat enum value. Defaults to Markdown if not specified.
     */
    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_H
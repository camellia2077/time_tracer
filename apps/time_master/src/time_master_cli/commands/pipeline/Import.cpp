// time_master_cli/commands/pipeline/Import.cpp
#include "Import.hpp"
#include <stdexcept>
#include <iostream>

Import::Import(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void Import::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'import' requires exactly one directory path argument.");
    }

    std::cout << "Now inserting into the database." << std::endl;
    std::cout << "Please ensure the data has been converted and validated." << std::endl;
    std::cout << "Are you sure you want to continue? (y/n): ";

    char confirmation;
    std::cin >> confirmation;
    if (confirmation != 'y' && confirmation != 'Y') {
        std::cout << "\033[31m\nOperation cancelled.\033[0m" << std::endl;
        return;
    }
    std::cout << std::endl;
    file_handler_.run_database_import(parser.get_filtered_args()[2]);
}
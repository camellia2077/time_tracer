#include <iostream>
#include <string>
#include <limits>
#include "Reprocessor.h"

// For UTF-8 output on Windows
#ifdef _WIN32
#include <windows.h>
#endif

// Sets up the console for proper UTF-8 character display.
void setup_console() {
#ifdef _WIN32
    // This is the primary and most reliable way to enable UTF-8 in the Windows console.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void print_menu() {
    std::cout << "\n===== Bill Reprocessor Menu =====\n";
    std::cout << "1. Validate Bill File\n";
    std::cout << "2. Modify Bill File\n";
    std::cout << "3. Exit\n";
    std::cout << "=================================\n";
    std::cout << "Enter your choice: ";
}

int main() {
    setup_console();

    std::cout << "Welcome to the Bill Reprocessor! (UTF-8 enabled)\n";
    std::cout << "Configuration will be loaded from the './config' directory.\n";

    try {
        // Automatically use the './config' directory.
        Reprocessor reprocessor("./config");

        int choice = 0;
        while (choice != 3) {
            print_menu();
            std::cin >> choice;

            // Handle non-numeric or invalid input
            if (std::cin.fail()) {
                std::cin.clear(); // Clear error flags
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
                std::cout << "Invalid input. Please enter a number.\n";
                choice = 0; // Reset choice to continue loop
                continue;
            }
            
            // Consume the rest of the line (the newline character)
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::string bill_path;

            switch (choice) {
                case 1: {
                    std::cout << "Enter path to bill file for validation: ";
                    std::getline(std::cin, bill_path);
                    if (!bill_path.empty()) {
                        reprocessor.validate_bill(bill_path);
                    } else {
                        std::cout << "Bill file path cannot be empty.\n";
                    }
                    break;
                }
                case 2: {
                    std::cout << "Enter path to source bill file for modification: ";
                    std::getline(std::cin, bill_path);

                    if (bill_path.empty()) {
                        std::cout << "Source bill file path cannot be empty.\n";
                        break;
                    }

                    // **MODIFIED**: Automatically generate the output path.
                    std::string filename;
                    // Find the last path separator ('/' or '\')
                    size_t last_slash_pos = bill_path.find_last_of("/\\");
                    if (std::string::npos != last_slash_pos) {
                        // If a separator is found, the filename is the substring after it.
                        filename = bill_path.substr(last_slash_pos + 1);
                    } else {
                        // Otherwise, the whole path is the filename.
                        filename = bill_path;
                    }
                    
                    std::string output_path = "modified_" + filename;

                    reprocessor.modify_bill(bill_path, output_path);
                    break;
                }
                case 3:
                    std::cout << "Exiting program. Goodbye!\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please select 1, 2, or 3.\n";
                    break;
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "\nA critical error occurred: " << e.what() << std::endl;
        return 1; // Exit with an error code
    }

    return 0;
}

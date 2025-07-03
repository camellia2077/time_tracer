/*
████████╗██╗███╗   ███╗███████╗    ███╗   ███╗ █████╗ ███████╗████████╗███████╗██████╗ 
╚══██╔══╝██║████╗ ████║██╔════╝    ████╗ ████║██╔══██╗██╔════╝╚══██╔══╝██╔════╝██╔══██╗
   ██║   ██║██╔████╔██║█████╗      ██╔████╔██║███████║███████╗   ██║   █████╗  ██████╔╝
   ██║   ██║██║╚██╔╝██║██╔══╝      ██║╚██╔╝██║██╔══██║╚════██║   ██║   ██╔══╝  ██╔══██╗
   ██║   ██║██║ ╚═╝ ██║███████╗    ██║ ╚═╝ ██║██║  ██║███████║   ██║   ███████╗██║  ██║
   ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝
                                                                                                                                                                                                          
*/

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <iostream>
#include <string>

#include "menu.h" 
#include "common_utils.h"     // Contains color macro definitions
#include "FileHandler.h"      // The new module for handling file operations

// Core constant for the database name
const std::string DATABASE_NAME = "time_data.db";

#if defined(_WIN32) || defined(_WIN64)
void EnableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

int main(int argc, char* argv[]) {
    #if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    EnableVirtualTerminalProcessing();
    #endif

    if (argc < 1) {
        std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": Cannot determine application path." << std::endl;
        return 1;
    }

    AppConfig app_config;
    std::string main_config_path_str;

    try {
        // 1. Instantiate the file handler with the executable's path
        FileHandler file_handler(argv[0]);
        
        // 2. Load all configurations. The method will throw an exception on failure.
        app_config = file_handler.load_configuration();
        
        // 3. Get the path to the main config file for passing to the menu if needed
        main_config_path_str = file_handler.get_main_config_path();

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Fatal Error" << RESET_COLOR << " during configuration setup: " << e.what() << std::endl;
        return 1;
    }

    // 4. Instantiate the menu, passing the database name, the fully populated AppConfig, and the main config path
    Menu app_menu(DATABASE_NAME, app_config, main_config_path_str);
    app_menu.run();
    
    return 0;
}
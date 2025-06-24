#ifndef MENU_H
#define MENU_H

#include <string>

// Forward-declare the sqlite3 struct to avoid including the full header here.
struct sqlite3;

/**
 * @class Menu
 * @brief Handles the main application menu and user interaction loop.
 */
class Menu {
public:
    /**
     * @brief Constructs the Menu object.
     * @param db_name The filename for the SQLite database.
     */
    explicit Menu(const std::string& db_name);

    /**
     * @brief Runs the main application loop.
     */
    void run();

private:
    // Member Variables
    sqlite3* db;
    std::string db_name_;

    // Private Methods
    void print_menu();
    bool handle_user_choice(int choice);
    bool open_database_if_needed();
    void close_database();

    // Input validation helpers
    std::string get_valid_date_input();
    std::string get_valid_month_input();
};

#endif // MENU_H
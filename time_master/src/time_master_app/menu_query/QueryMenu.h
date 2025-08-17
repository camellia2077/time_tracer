// time_master_app/menu_query/QueryMenu.h
#ifndef QUERY_MENU_H
#define QUERY_MENU_H

#include <memory>

class ReportGenerationHandler;

class QueryMenu {
public:
    explicit QueryMenu(ReportGenerationHandler* handler);
    void run();

private:
    ReportGenerationHandler* report_generation_handler_;

    void print_menu() const;
    void handle_choice(int choice);
    void run_period_query_prompt();
};

#endif // QUERY_MENU_H
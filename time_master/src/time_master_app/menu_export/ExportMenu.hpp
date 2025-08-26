// time_master_app/menu_export/ExportMenu.hpp
#ifndef EXPORT_MENU_HPP
#define EXPORT_MENU_HPP

#include <memory>

class ReportGenerationHandler;

class ExportMenu {
public:
    explicit ExportMenu(ReportGenerationHandler* handler);
    void run();

private:
    ReportGenerationHandler* report_generation_handler_;

    void print_menu() const;
    void handle_choice(int choice);
    
    // --- Single export functions ---
    void run_export_single_day_report_prompt();
    void run_export_single_month_report_prompt();
    void run_export_single_period_report_prompt();

    // --- Bulk export functions ---
    void run_export_all_period_reports_prompt();
};

#endif // EXPORT_MENU_HPP
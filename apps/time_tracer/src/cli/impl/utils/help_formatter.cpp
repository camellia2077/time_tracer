// cli/impl/utils/help_formatter.cpp
#include "help_formatter.hpp"
#include <iostream>
#include <print>
#include "common/AnsiColors.hpp"

void print_full_usage(const char* app_name) {
    std::println("{}TimeTracer{}: A command-line tool for time data pre-processing, import, and querying.\n",GREEN_COLOR,RESET_COLOR);
    std::println("Usage: {} <command>[arguments...] [options...]\n", app_name);
    
    std::println("{}{}{}", GREEN_COLOR, "--- Core Commands ---", RESET_COLOR);
    std::println("   run-pipeline, blink <path>\t Run full pipeline: validate, convert, validate output, and import to database.");

    std::println("                                  Options:");
    std::println("                                    --date-check <mode>     Set date check mode: 'continuity' or 'full'.");
    std::println("                                    --no-date-check         Disable date checking (overrides config).");
    std::println("                                    --save-processed        Force save intermediate JSON files.");
    std::println("                                    --no-save               Force NOT to save intermediate JSON files (overrides config).");

    std::println("  query <type> <period>\t\t Query data from the database.");
    std::println("  export <type> <period>\t Export reports from the database.\n");

    // 预处理
    std::println("{}{}{}", GREEN_COLOR, "--- Pre-processing Commands ---", RESET_COLOR);
    std::println("  validate-source <path>\t Validates the source file format (e.g., .txt files) (read-only).");
    std::println("  convert <path>\t\t Converts source files to the processed JSON format.");
    std::println("  validate-output <path>\t Validates the processed JSON file format and logic (read-only).");
    std::println("  \tNote: Validation commands do not produce output; the --output option has no effect on them.");
    
    std::println("  Options for validate-output:");
    std::println("    --date-check <mode>\t\t Check date logic. Modes:");
    std::println("                                    continuity : Check for missing dates between records.");
    std::println("                                    full       : Check for all dates in the month (completeness).");
    std::println("                                    none       : Disable check explicitly.");

    std::println("  Example: {} convert /path/to/logs", app_name);
    std::println("  Example: {} validate-output /path/to/processed/log.json --date-check full\n", app_name);

    // 数据库导入
    std::println("{}{}{}", GREEN_COLOR, "--- Command: import ---", RESET_COLOR);
    std::println("  Usage: {} import <directory_path>", app_name);
    std::println("  Example: {} import /path/to/output/Processed_logs/\n", app_name);
    
    // 查询与导出
    std::println("{}{}{}", GREEN_COLOR, "--- Command: query ---", RESET_COLOR);
    std::println("  Usage: {} query <type> <argument> [options...]", app_name);
    std::println("  Types:");
    std::println("    daily <YYYYMMDD>\t\t Query statistics for a specific day.");
    std::println("    monthly <YYYYMM>\t\t Query statistics for a specific month.");
    std::println("    period <days>\t\t Query statistics for last N days. Can be a list (e.g., 7,30).");
    std::println("  Options:");
    std::println("    --format, -f <format>\t Specify output format (md, tex, typ).");
    std::println("    --db, --database <path>\t Specify the database file to query."); // [新增]
    std::println("                         \t Default: [program_location]/output/time_data.sqlite3");
    std::println("  Example: {} query daily 20240101 --db ./my_data.sqlite3\n", app_name);
    
    // 导出
    std::println("{}{}{}", GREEN_COLOR, "--- Command: export ---", RESET_COLOR);
    std::println("  Usage: {} export <type> [argument] [options...]", app_name);
    std::println("  Types:");
    std::println("    daily <YYYYMMDD>\t\t Export a single daily report.");
    std::println("    monthly <YYYYMM>\t\t Export a single monthly report.");
    std::println("    period <days>\t\t Export a single period report (e.g., 7).");
    std::println("    all-daily\t\t\t Export all daily reports.");
    std::println("    all-monthly\t\t\t Export all monthly reports.");
    std::println("    all-period <days_list>\t Export multiple period reports (e.g., 7,30,90).");
    std::println("  Options:");
    std::println("    --format, -f <format>\t Specify output format for query/export (md, tex, typ). Default: md.");
    std::println("    --output, -o <path>\t\t Specify the directory for generated report files."); // [修改]
    std::println("                         \t Default: [program_location]/output/exported_files");
    std::println("    --db, --database <path>\t Specify the database file to read data from."); // [新增]
    std::println("                         \t Default: [program_location]/output/time_data.sqlite3");
    
    std::println("  Example: {} export daily 20240115 -f tex -o /my/reports", app_name);
    std::println("  Example: {} export all-monthly --db /backups/2024.sqlite3\n", app_name);

    std::println("{}{}{}", GREEN_COLOR, "--- Other Options ---", RESET_COLOR);
    std::println("  --help, -h\t\t\t Show this help message.");
    std::println("  --version, -v\t\t\t Show program version.");
}
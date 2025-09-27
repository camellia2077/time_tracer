// action_handler/query/QueryManager.hpp

#ifndef QUERY_MANAGER_HPP
#define QUERY_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <memory> // For std::unique_ptr
#include "reports/shared/types/ReportFormat.hpp" 
#include "common/AppConfig.hpp" 



// Forward Declaration
class QueryHandler;

class QueryManager {
public:
    explicit QueryManager(sqlite3* db, const AppConfig& config); // [MODIFIED] Constructor
    ~QueryManager();

    std::string run_daily_query(const std::string& date, ReportFormat format);
    std::string run_monthly_query(const std::string& month, ReportFormat format);
    std::string run_period_query(int days, ReportFormat format);

private:
    const AppConfig& app_config_; // [ADDED] Store a reference to the config
    std::unique_ptr<QueryHandler> query_handler_;
};

#endif // QUERY_MANAGER_HPP
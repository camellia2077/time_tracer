// action_handler/query/QueryManager.cpp

#include "QueryManager.hpp"
#include "queries/QueryHandler.hpp" // Include the underlying query processor

QueryManager::QueryManager(sqlite3* db, const AppConfig& config) 
    : app_config_(config) { // [MODIFIED] Initialize the config reference
    // Create a QueryHandler instance and pass the config to it
    query_handler_ = std::make_unique<QueryHandler>(db, app_config_);
}

QueryManager::~QueryManager() = default;

std::string QueryManager::run_daily_query(const std::string& date, ReportFormat format) {
    return query_handler_->run_daily_query(date, format);
}

std::string QueryManager::run_monthly_query(const std::string& month, ReportFormat format) {
    return query_handler_->run_monthly_query(month, format);
}

std::string QueryManager::run_period_query(int days, ReportFormat format) {
    return query_handler_->run_period_query(days, format);
}
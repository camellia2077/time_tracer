// action_handler/query/DirectQueryManager.cpp

#include "DirectQueryManager.hpp"
#include "queries/QueryHandler.hpp" // Include the underlying query processor

DirectQueryManager::DirectQueryManager(sqlite3* db, const AppConfig& config) 
    : app_config_(config) { // [MODIFIED] Initialize the config reference
    // Create a QueryHandler instance and pass the config to it
    query_handler_ = std::make_unique<QueryHandler>(db, app_config_);
}

DirectQueryManager::~DirectQueryManager() = default;

std::string DirectQueryManager::run_daily_query(const std::string& date, ReportFormat format) {
    return query_handler_->run_daily_query(date, format);
}

std::string DirectQueryManager::run_monthly_query(const std::string& month, ReportFormat format) {
    return query_handler_->run_monthly_query(month, format);
}

std::string DirectQueryManager::run_period_query(int days, ReportFormat format) {
    return query_handler_->run_period_query(days, format);
}
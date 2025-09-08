// db_inserter/inserter/DataInserter.hpp
#ifndef DATA_INSERTER_HPP
#define DATA_INSERTER_HPP

#include <vector>
#include <unordered_set>
#include <string>
#include <utility>
#include <sqlite3.h>
#include "db_inserter/model/time_sheet_model.hpp"

class DataInserter {
public:
    explicit DataInserter(sqlite3* db, sqlite3_stmt* stmt_day, sqlite3_stmt* stmt_record, sqlite3_stmt* stmt_parent_child);

    void insert_days(const std::vector<DayData>& days);
    void insert_records(const std::vector<TimeRecordInternal>& records);
    void insert_parent_child_pairs(const std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs);

private:
    sqlite3* db;
    sqlite3_stmt* stmt_insert_day;
    sqlite3_stmt* stmt_insert_record;
    sqlite3_stmt* stmt_insert_parent_child;
};

#endif // DATA_INSERTER_HPP
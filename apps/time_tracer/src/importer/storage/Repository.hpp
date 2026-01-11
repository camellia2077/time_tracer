// importer/storage/Repository.hpp
#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <string>
#include <vector>
#include <memory>
#include "importer/model/TimeSheetData.hpp"

// 引用新的组件头文件
#include "importer/storage/sqlite/Writer.hpp"
#include "importer/storage/sqlite/Connection.hpp"
#include "importer/storage/sqlite/Statement.hpp"

/**
 * @class Repository
 * @brief Facade class that simplifies the process of importing data into the database.
 */
class Repository {
public:
    explicit Repository(const std::string& db_path);
    ~Repository() = default;

    bool is_db_open() const;

    void import_data(
        const std::vector<DayData>& days,
        const std::vector<TimeRecordInternal>& records
    );

private:
    std::unique_ptr<Connection> connection_manager_;
    std::unique_ptr<Statement> statement_manager_; 
    std::unique_ptr<Writer> data_inserter_;        
};

#endif // REPOSITORY_HPP
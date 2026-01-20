// reports/data/interfaces/i_project_info_provider.hpp
#ifndef REPORTS_DATA_INTERFACES_I_PROJECT_INFO_PROVIDER_HPP_
#define REPORTS_DATA_INTERFACES_I_PROJECT_INFO_PROVIDER_HPP_

#include <vector>
#include <string>
#include <sqlite3.h>

// [修改] 移除 REPORTS_DATA_API 宏，直接声明类
class IProjectInfoProvider {
public:
    virtual ~IProjectInfoProvider() = default;

    // 确保缓存数据已加载
    virtual void ensure_loaded(sqlite3* db) = 0;

    // 获取项目路径部分
    virtual std::vector<std::string> get_path_parts(long long project_id) const = 0;
};

#endif // REPORTS_DATA_INTERFACES_I_PROJECT_INFO_PROVIDER_HPP_
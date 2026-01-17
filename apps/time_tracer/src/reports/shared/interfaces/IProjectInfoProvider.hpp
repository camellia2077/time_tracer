// reports/shared/interfaces/IProjectInfoProvider.hpp
#ifndef REPORTS_SHARED_INTERFACES_I_PROJECT_INFO_PROVIDER_HPP_
#define REPORTS_SHARED_INTERFACES_I_PROJECT_INFO_PROVIDER_HPP_

#include <vector>
#include <string>
#include <sqlite3.h>
#include "reports/shared/api/shared_api.hpp"

class REPORTS_SHARED_API IProjectInfoProvider {
public:
    virtual ~IProjectInfoProvider() = default;

    // 确保缓存数据已加载
    virtual void ensure_loaded(sqlite3* db) = 0;

    // 获取项目路径部分
    virtual std::vector<std::string> get_path_parts(long long project_id) const = 0;
};

#endif // REPORTS_SHARED_INTERFACES_I_PROJECT_INFO_PROVIDER_HPP_
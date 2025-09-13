// queries/shared/Interface/ITreeFmt.hpp
#ifndef I_PROJECT_BREAKDOWN_FORMATTER_HPP
#define I_PROJECT_BREAKDOWN_FORMATTER_HPP

#include <string>
#include <sstream>
#include "common/utils/ProjectTree.hpp" // For ProjectNode, ProjectTree" // For ProjectTree

/**
 * @class ITreeFmt
 * @brief 项目明细格式化器的通用接口（抽象基类）。
 */
class ITreeFmt {
public:
    virtual ~ITreeFmt() = default;

    /**
     * @brief 格式化项目明细数据。
     * @param tree 包含项目结构和时间的树。
     * @param total_duration 用于计算百分比的总时间。
     * @param avg_days 用于格式化时间的平均天数。
     * @return 一个表示完整报告的格式化字符串。
     */
    virtual std::string format(const ProjectTree& tree, long long total_duration, int avg_days) const = 0;
};

#endif // I_PROJECT_BREAKDOWN_FORMATTER_HPP
// queries/shared/breakdown/formatters/BreakdownTyp.h
#ifndef PROJECT_BREAKDOWN_TYP_FORMAT_HPP
#define PROJECT_BREAKDOWN_TYP_FORMAT_HPP

#include "queries/shared/interface/ITreeFmt.hpp"

class BreakdownTyp : public ITreeFmt {
public:
    std::string format(const ProjectTree& tree, long long total_duration, int avg_days) const override;

private:
    // 递归辅助函数，用于生成层级的输出
    void generate_sorted_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const;
};

#endif // PROJECT_BREAKDOWN_TYP_FORMAT_HPP
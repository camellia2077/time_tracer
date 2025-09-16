// queries/export/AllDayReports.hpp
#ifndef ALL_REPORTS_GENERATOR_HPP
#define ALL_REPORTS_GENERATOR_HPP 

#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp" 

#include <sqlite3.h>
#include <string>

class AllDayReports {
public:
    /**
     * @brief 构造函数。
     * @param db 指向数据库连接的指针。
     * @param day_typ_config_path DayTyp 配置文件的路径。
     * @param day_md_config_path DayMd 配置文件的路径。
     * @param day_tex_config_path [新增] DayTex 配置文件的路径。
     */
    explicit AllDayReports(sqlite3* db, const std::string& day_typ_config_path, const std::string& day_md_config_path, const std::string& day_tex_config_path);

    /**
     * @brief 生成所有日报并返回分类好的结果。
     * @param format 需要生成的报告格式。
     * @return 一个包含所有格式化后日报的嵌套 map。
     */
    FormattedGroupedReports generate_all_reports(ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_day_typ_config_path;
    std::string m_day_md_config_path;
    std::string m_day_tex_config_path; // [新增]
};

#endif // ALL_REPORTS_GENERATOR_HPP
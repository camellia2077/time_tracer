// queries/shared/formatters/tex/BaseTexFormatter.hpp
#ifndef BASE_TEX_FORMATTER_HPP
#define BASE_TEX_FORMATTER_HPP

#include <sstream>
#include <string>

// --- Forward Declarations ---
struct DailyReportData;
struct MonthlyReportData;
struct PeriodReportData;
struct sqlite3;

class BaseTexFormatter {
public:
    virtual ~BaseTexFormatter() = default;

protected:
    template<typename ReportData>
    std::string format_report_template(const ReportData& data, sqlite3* db) const {
        std::stringstream ss;
        ss << get_tex_preamble();
        format_content(ss, data, db);
        ss << get_tex_postfix();
        return ss.str();
    }

    virtual void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const = 0;
    virtual void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const = 0;
    virtual void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const = 0;

private:
    std::string get_tex_preamble() const {
        std::stringstream ss;
        ss << "\\documentclass{article}\n";
        ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
        ss << "\\usepackage[dvipsnames]{xcolor}\n";
        ss << "\\usepackage{enumitem}\n";
        ss << "\\usepackage{fontspec}\n";
        ss << "\\usepackage{ctex}\n";
        ss << "\\definecolor{studycolor}{HTML}{2ECC40}\n";
        ss << "\\definecolor{recreationcolor}{HTML}{FF4136}\n";
        ss << "\\definecolor{mealcolor}{HTML}{FF851B}\n";
        ss << "\\definecolor{exercisecolor}{HTML}{0074D9}\n";
        ss << "\\definecolor{routinecolor}{HTML}{AAAAAA}\n";
        ss << "\\definecolor{sleepcolor}{HTML}{B10DC9}\n";
        ss << "\\definecolor{codecolor}{HTML}{39CCCC}\n\n";
        ss << "\\setmainfont{Noto Serif SC}\n";
        ss << "\\setCJKmainfont{Noto Serif SC}\n\n";
        ss << "\\begin{document}\n\n";
        return ss.str();
    }

    std::string get_tex_postfix() const {
        return "\n\\end{document}\n";
    }
};

#endif // BASE_TEX_FORMATTER_HPP
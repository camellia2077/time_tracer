// queries/_shared/BaseTexFormatter.h
#ifndef BASE_TEX_FORMATTER_H
#define BASE_TEX_FORMATTER_H

#include <sstream>
#include <string>
//提供tex中固定内容的基类

// --- 前置声明 (Forward Declarations) ---
// 为了让这个头文件更独立、编译更快，我们只声明需要用到的类型，
// 而不是包含它们的完整头文件。
struct DailyReportData;
struct MonthlyReportData;
struct PeriodReportData;
struct sqlite3;

/**
 * @class BaseTexFormatter
 * @brief 所有 TeX 报告格式化器的抽象基类，使用模板方法模式。
 *
 * 这个基类负责定义生成一个完整 TeX 文档的通用流程（算法骨架），
 * 包括文档头(Preamble)的生成和文档尾的添加。
 *
 * 它将“如何生成报告的核心内容”这一具体步骤，通过纯虚函数 format_content
 * 延迟到其子类（如 DailyTex, MonthlyTex）中去实现。
 */
class BaseTexFormatter {
public:
    // 提供一个虚析构函数是作为基类的良好实践
    virtual ~BaseTexFormatter() = default;

protected:
    // --- 模板方法 (Template Method) ---
    // 这个函数是protected的，因为它只应该被子类调用。
    // 它定义了生成报告的固定步骤，是不可被子类重写的。
    template<typename ReportData>
    std::string format_report_template(const ReportData& data, sqlite3* db) const {
        std::stringstream ss;

        // 步骤 1: 写入所有 TeX 文档共用的“文档头”
        ss << get_tex_preamble();
        
        // 步骤 2: 调用一个由子类具体实现的“钩子”函数来填充核心内容
        format_content(ss, data, db);
        
        // 步骤 3: 写入所有 TeX 文档共用的“文档尾”
        ss << get_tex_postfix();
        
        return ss.str();
    }

    // --- “钩子”函数 (Hook Functions) ---
    // 这些是纯虚函数，子类必须根据自己处理的数据类型来实现它们。
    // 我们为每种报告数据类型都提供一个重载版本。
    virtual void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const = 0;
    virtual void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const = 0;
    virtual void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const = 0;

private:
    // --- 基类提供的具体实现 ---
    // 这两个私有函数是通用框架的一部分，对所有子类都可用。
    
    // 生成 TeX 文档的“文档头” (Preamble)
    std::string get_tex_preamble() const {
        std::stringstream ss;
        ss << "\\documentclass{article}\n";
        ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
        ss << "\\usepackage{fontspec}\n";
        ss << "\\usepackage{ctex}\n";
        // 将中英文字体分开设置
        ss << "\\setmainfont{Noto Serif SC}\n";      // 用于英文、数字
        ss << "\\setCJKmainfont{Noto Serif SC}\n\n"; // 用于中文字符
        ss << "\\begin{document}\n\n";
        return ss.str();
    }

    // 生成 TeX 文档的“文档尾”
    std::string get_tex_postfix() const {
        return "\n\\end{document}\n";
    }
};

#endif // BASE_TEX_FORMATTER_H